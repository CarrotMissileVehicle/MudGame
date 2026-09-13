/**
 * @file WorldEngine.cpp
 * @brief WorldEngine 实现：世界推进与采矿/钓鱼动作结算（源自原 main.cpp）。
 */
#include "WorldEngine.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>

#include "GameContext.h"

WorldEngine::WorldEngine(GameContext& ctx) : ctx_(ctx) {}

// 采矿上下文生成：由玩家采矿经验推导等级，由矿镐等级推导速度与间隔
mining::MiningContext WorldEngine::build_mine_ctx() const
{
    mining::MiningContext mc;
    mc.mining_level = 1 + static_cast<std::size_t>(ctx_.player.GetMineExp() / 100);
    mc.has_torch = true;      // 演示：默认持有照明
    mc.has_lantern = true;
    const int pick = ctx_.tools.level(mud::tool::ToolId::Pickaxe);
    switch (pick) {
        case 5: mc.tool.mining_speed = mining::MiningSpeed::Iron;    break;
        case 4: mc.tool.mining_speed = mining::MiningSpeed::Silver;  break;
        case 3: mc.tool.mining_speed = mining::MiningSpeed::Gold;    break;
        case 2: mc.tool.mining_speed = mining::MiningSpeed::Crystal; break;
        default: mc.tool.mining_speed = mining::MiningSpeed::Core;   break;
    }
    mc.tool.interval = std::max<std::int64_t>(1, 7 - pick); // 矿镐等级越高，单次采矿间隔越短
    return mc;
}

// 采矿产出入库：按矿石数据复制新 Object 进背包，并累计采矿经验；
// 反馈文本统一收集后交由 MiningPanel 渲染。
void WorldEngine::grant_mining(const std::vector<mining::MiningResult>& results, bool render)
{
    std::vector<mud::view::MiningEventView> events;
    for (const auto& r : results) {
        const std::string name = ctx_.ore_table.get_ore_name(r.ore_id);
        const std::string usage = ctx_.ore_table.get_ore_usage(r.ore_id);
        const auto price = ctx_.ore_table.get_ore_price(r.ore_id);
        for (std::size_t i = 0; i < r.quantity; ++i)
            ctx_.player.GetBag().AddObject(new Object(name, usage, 0,
                 static_cast<int>(price), 0));
        ctx_.player.SetMineExp(ctx_.player.GetMineExp() + static_cast<int>(r.experience));
        events.push_back({name, r.quantity, r.experience});
    }
    if (render && !events.empty())
        ctx_.view.render_mining_produce(events);
}

// 结束当前钓鱼动作：复位玩家状态并清空 TUI 动作状态。
void WorldEngine::end_fishing(bool manual)
{
    ctx_.player.SetState(StateCode::Waiting);
    if (manual)
        ctx_.msg("钓鱼结束（手动退出），共钓到 " + std::to_string(ctx_.tui.action_cycles) + " 条鱼。");
    else
        ctx_.msg("钓鱼结束，共钓到 " + std::to_string(ctx_.tui.action_cycles) + " 条鱼。");
    ctx_.tui.action_type = mud::tui::ActionType::None;
    ctx_.tui.action_cycles = 0;
    ctx_.tui.action_next_ms = 0;
}

// 由 1 秒后台节拍调用的动作推进：钓鱼按记录的时间点逐轮结算。
void WorldEngine::handle_action_tick()
{
    if (ctx_.tui.action_type != mud::tui::ActionType::Fish)
        return;
    const auto now = std::chrono::steady_clock::now();
    const std::int64_t now_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    if (now_ms < ctx_.tui.action_next_ms)
        return;
    if (ctx_.player.GetSatiety() <= 0) {
        ctx_.msg("体力耗尽，钓鱼停止。");
        end_fishing(false);
        return;
    }
    ctx_.player.SetSatiety(ctx_.player.GetSatiety() - kFishingSatietyCost);
    Fish* f = ctx_.fishing.tickFish();
    if (f == nullptr) {
        ctx_.msg("  这一轮没有钓到鱼。");
    } else {
        const std::string fname = ctx_.fish_names.at(f);
        ctx_.player.GetBag().AddObject(new Object(fname, "刚钓上来的鱼", 0,
            f->GetSellingPrice(), f->GetBuyingPrice()));
        ctx_.player.SetFishExp(ctx_.player.GetFishExp() + f->getFishExp());
        ctx_.msg("  钓到 " + fname + "！");
    }
    ++ctx_.tui.action_cycles;
    ctx_.tui.action_next_ms = now_ms + kActionWaitMinMs
        + std::rand() % (kActionWaitMaxMs - kActionWaitMinMs + 1);
}

// 世界推进（后台线程每现实秒调用）：经 TimeService::update() 按 time_scale
// 推进游戏时间，随后对实际跨过的每个游戏分钟驱动天气/作物/钓鱼，
// 跨天刷新集市；若在采矿则按到期进度统一结算产出。
// 注意：time_scale 可非整分钟（如 0.5），整分钟未凑满时跳过本次推进。
void WorldEngine::tick_world()
{
    const auto before = ctx_.time.now();
    ctx_.time.update();
    const std::int64_t before_total = before.total_minutes();
    const std::int64_t now_total = ctx_.time.now().total_minutes();
    if (now_total <= before_total) return; // 亚分钟未凑满整分钟，无需推进

    std::int64_t last_day = before_total / 1440;
    auto cursor = before;
    for (std::int64_t t = before_total; t < now_total; ++t) {
        cursor.advance(1);
        ctx_.weather.update();
        ctx_.farming.tick(cursor);
        ctx_.fishing.tick(cursor);
        const std::int64_t day = cursor.total_minutes() / 1440;
        if (day != last_day) {
            ctx_.market.onNewDay(cursor);
            last_day = day;
        }
    }
    if (ctx_.mining.is_mining()) {
        const auto results = ctx_.mining.poll(build_mine_ctx());
        grant_mining(results, false); // 后台静默入包，不打断正在键入的命令
    }
}