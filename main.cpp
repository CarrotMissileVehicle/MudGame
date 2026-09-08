/**
 * @file main.cpp
 * @brief MUD 游戏命令行入口：组合根（composition root）。
 *
 * 职责：
 *   1. 按构造函数注入装配全部子系统（玩家/农田/钓鱼/市场/天气/采矿/工具）
 *   2. 以"回合制"REPL 驱动游戏循环：解析命令 → 派发领域 handler
 *   3. 时间由后台线程按 time_scale 随真实时间自动推进，逐分钟联动天气/
 *      作物/钓鱼/集市跨天与采矿结算（无需手动 time.tick）
 *
 * 并发：全局 worldMutex 串行化「后台时间推进」与「REPL 命令执行」，
 * 二者互斥访问共享的世界状态，避免数据竞争。
 *
 * 渲染：本文件不再直接使用 std::cout。
 * 所有输出统一经由 StdoutRenderer → TerminalView（装配 DTO 后渲染）；
 * 命令反馈文本通过 MessagePanel 展示，帮助文本通过 HelpScreen 展示。
 */
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include <chrono>
#include <mutex>
#include <thread>

#include <windows.h>
#include <conio.h>   // _kbhit / _getch：前台等待循环检测 q 键

#include "Game.h"
#include "time_service.h"
#include "Player.h"
#include "Move.h"

// 农田 / 作物 / 肥料 / 钓鱼 / 市场（编译进 MudGame 目标的 JERRY_SOURCES）
#include "farm.h"
#include "farmland.h"
#include "FarmingController.h"
#include "Cabbage.h"
#include "Carrot.h"
#include "Tomato.h"
#include "Punpkin.h"
#include "Lingzhi.h"
#include "NormalFertilizer.h"
#include "AdvancedFertilizer.h"
#include "Fish.h"
#include "Crucian.h"
#include "GrassCarp.h"
#include "Perch.h"
#include "RainbowTrout.h"
#include "KingCrab.h"
#include "FishingController.h"
#include "Market.h"
#include "Shop.h"
#include "ShopItem.h"

// 天气 / 工具 / 采矿 / 视图 / 命令解析
#include "weather_controller.h"
#include "tool_controller.h"
#include "mining_controller.h"
#include "mining_handler.h"
#include "mining_types.h"
#include "event.h"
#include "Renderer.h"
#include "dto.h"
#include "TerminalView.h"
#include "input_parser.h"
#include "connector.h"
#include "PlayerSerializer.h"

namespace
{
    // 每次试探性行动（钓鱼/采矿）等待的随机时间范围（毫秒）
    constexpr int kActionWaitMinMs = 3000;
    constexpr int kActionWaitMaxMs = 6000;

    // 每次垂钓消耗的体力（饱食度）点数
    constexpr int kFishingSatietyCost = 5;

    // 默认存档文件名
    constexpr const char* kSaveFileName = "mudgame.sav";

    // 从命令 options 读取整数型命名参数，缺省返回 fallback。
    long long opt_int(const mud::cmd::Command& cmd, const std::string& key, long long fallback)
    {
        const auto it = cmd.options.find(key);
        if (it == cmd.options.end())
            return fallback;
        try { return std::stoll(it->second); }
        catch (...) { return fallback; }
    }
} // namespace

int main()
{
    SetConsoleOutputCP(65001);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Game game;
    game.startSession();

    // ---- 时间（唯一时间真相）----
    mud::TimeService timeService(mud::time::GameDateTime{0, 1, 1, 8, 0});

    // ---- 玩家 ----
    Player player;
    Move move(player);

    // ---- 农田 ----
    Farm farm({FarmLand(), FarmLand(), FarmLand(), FarmLand()});
    FarmingController farming(&farm);

    // ---- 作物/肥料原型（堆栈持有，地块仅保存非拥有指针）----
    Cabbage cabbage; Carrot carrot; Tomato tomato; Pumpkin pumpkin; Lingzhi lingzhi;
    NormalFertilizer normalFert; AdvancedFertilizer advancedFert;

    const std::map<std::string, Crop*> kSeeds = {
        {"cabbage", &cabbage}, {"carrot", &carrot}, {"tomato", &tomato},
        {"pumpkin", &pumpkin}, {"lingzhi", &lingzhi}};
    const std::map<const Crop*, std::string> kCropNames = {
        {&cabbage, "小白菜"}, {&carrot, "胡萝卜"}, {&tomato, "番茄"},
        {&pumpkin, "南瓜"},  {&lingzhi, "灵芝"}};

    // ---- 鱼池（堆栈持有，钓到即复制入背包，绝不共享指针）----
    Crucian crucian; GrassCarp grassCarp; Perch perch; RainbowTrout rainbowTrout; KingCrab kingCrab;
    const std::vector<Fish*> kFishPool = {&crucian, &grassCarp, &perch, &rainbowTrout, &kingCrab};
    const std::map<const Fish*, std::string> kFishNames = {
        {&crucian, "小鲫鱼"}, {&grassCarp, "草鱼"}, {&perch, "鲈鱼"},
        {&rainbowTrout, "虹鳟鱼"}, {&kingCrab, "帝王蟹"}};
    FishingController fishing(kFishPool, 0.3f);

    // ---- 集市 ----
    // 商店货架商品：独立的具名 Object 实例（价格对齐对应作物/肥料），
    // 生命周期与 main 一致；购买会向背包复制新对象。
    Object seedCabbage{"小白菜种子", "种下后收获小白菜", 0, 8, 5};
    Object seedCarrot{"胡萝卜种子", "种下后收获胡萝卜", 0, 10, 8};
    Object seedTomato{"番茄种子", "种下后收获番茄", 0, 12, 10};
    Object seedPumpkin{"南瓜种子", "种下后收获南瓜", 0, 15, 15};
    Object seedLingzhi{"灵芝孢子", "种下后收获灵芝", 0, 20, 25};
    Object fertNormal{"普通肥料", "生长周期减半", 0, 10, 15};
    Object fertAdvanced{"高级肥料", "生长周期加速", 0, 25, 30};
    Market market;
    Shop seedShop("seed", "种子商店");
    seedShop.addItem(ShopItem(&seedCabbage));
    seedShop.addItem(ShopItem(&seedCarrot));
    seedShop.addItem(ShopItem(&seedTomato));
    seedShop.addItem(ShopItem(&seedPumpkin));
    seedShop.addItem(ShopItem(&seedLingzhi));
    Shop groceryShop("grocery", "杂货铺");
    groceryShop.addItem(ShopItem(&fertNormal));
    groceryShop.addItem(ShopItem(&fertAdvanced));
    // 铁匠铺：位于集市，专营工具修复服务（修复按丢失的矿石/金币扣费，不由货架商品表达）
    Shop blacksmithShop("blacksmith", "铁匠铺");
    market.registerShop(seedShop);
    market.registerShop(groceryShop);
    market.registerShop(blacksmithShop);
    market.onNewDay(timeService.now()); // 同步集市日历到当前游戏时间
    int gold = 200;

    // ---- 天气 ----
    WeatherController weather(timeService, farm);

    // ---- 工具 / 采矿 ----
    mud::tool::ToolController tools;
    Ore::OreData oreData{};           // 控制器仅持引用、无需数据文件内容
    Ore oreTable;                     // 供玩家反馈名称/价格/用途
    mud::event::EventSystem miningEvents;
    MiningController miningController(oreData, timeService, &miningEvents, &tools);
    MiningHandler miningHandler(miningController);

// ---- 视图（View 层唯一输出出口）----
    mud::view::StdoutRenderer outRenderer;
    mud::view::TerminalView terminal(outRenderer);

    // ---- 解析 / 派发 ----
    InputParser parser;
    Connector connector;
    ParameterCollector paramCollector(outRenderer);
    connector.set_collector(&paramCollector);
    HandlerContext ctx{timeService, miningHandler};

    // ================= DTO 装配（组合根 → View 的唯一数据通道）=================
    const auto make_time_view = [&]() -> mud::view::TimeView {
        const auto now = timeService.now();
        mud::view::TimeView t;
        t.year = now.year; t.month = now.month; t.day = now.day;
        t.hour = now.hour; t.minute = now.minute;
        t.time_scale = timeService.time_scale();
        return t;
    };

    const auto make_weather_view = [&]() -> mud::view::WeatherView {
        mud::view::WeatherView w;
        w.weather = weather.weather_name();
        w.can_fish = weather.can_fish();
        w.can_go_outside = weather.can_go_outside();
        w.auto_water = weather.auto_water();
        w.crop_loss_rate = weather.crop_loss_rate();
        w.mining_exp_bonus = weather.mining_exp_bonus();
        w.fishing_penalty = weather.fishing_penalty();
        w.events = weather.today_event_names();
        return w;
    };

    const auto make_player_status = [&]() -> PlayerStatus {
        PlayerStatus s;
        s.position = player.GetPosition();
        s.state = player.GetState();
        s.satiety = player.GetSatiety();
        s.maxSatiety = player.GetMaxSatiety();
        s.farmingExp = player.GetFarmingExp();
        s.fishExp = player.GetFishExp();
        s.mineExp = player.GetMineExp();
        s.bagItems = player.GetBag().GetStackedNames();
        return s;
    };

    const auto make_farm_view = [&]() -> mud::view::FarmView {
        mud::view::FarmView f;
        for (std::size_t i = 0; i < farming.farmSize(); ++i)
        {
            const auto& fl = farm.getFarmland(i);
            mud::view::PlotView p;
            p.index = i;
            p.occupied = fl.isOccupied();
            if (fl.isOccupied())
            {
                const std::string cname = kCropNames.count(fl.getCrop())
                    ? kCropNames.at(fl.getCrop()) : "未知作物";
                p.crop_name = cname;
                const int growthCycle = fl.getCrop()->getGrowthCycle();
                p.growth_stage = fl.getGrowthStage() > growthCycle
                    ? growthCycle : fl.getGrowthStage(); // 防御性钳制
                p.growth_max = growthCycle;
                p.watered = fl.isWatered();
            }
            f.plots.push_back(p);
        }
        return f;
    };

    const auto make_fishing_view = [&]() -> mud::view::FishingView {
        mud::view::FishingView f;
        for (const Fish* fish : kFishPool)
            f.pool.push_back({kFishNames.at(fish), fish->getProbability()});
        f.can_fish = weather.can_fish();
        return f;
    };

    const auto make_market_view = [&]() -> mud::view::MarketView {
        mud::view::MarketView m;
        m.gold = gold;
        m.day_of_week = market.getDayOfWeek();
        m.prosperous = market.isProsperousDay();
        m.festival = market.isFestival();
        for (std::size_t i = 0; i < market.shopCount(); ++i)
        {
            const Shop& shop = market.getShop(i);
            mud::view::ShopView sv;
            sv.id = shop.getId();
            sv.name = shop.getName();
            for (std::size_t j = 0; j < shop.itemCount(); ++j)
            {
                Object* item = shop.getItem(j).getItem();
                if (item == nullptr) continue;
                mud::view::MarketItemView iv;
                iv.name = item->GetName();
                iv.buy = market.getBuyPrice(shop.getId(), item);
                iv.sell = market.getSellPrice(item);
                sv.items.push_back(iv);
            }
            m.shops.push_back(sv);
        }
        return m;
    };

    const auto make_tools_view = [&]() -> mud::view::ToolsView {
        mud::view::ToolsView t;
        for (const auto id :
             {mud::tool::ToolId::Hoe, mud::tool::ToolId::Rod, mud::tool::ToolId::Pickaxe})
        {
            mud::view::ToolView tv;
            tv.name = tools.name(id);
            tv.durability = tools.durability(id);
            tv.level = tools.level(id);
            tv.broken = tools.is_broken(id);
            t.tools.push_back(tv);
        }
        return t;
    };

    const auto make_mining_view = [&]() -> mud::view::MiningView {
        mud::view::MiningView m;
        m.is_mining = miningHandler.is_mining();
        m.layer = miningHandler.layer_id() ? *miningHandler.layer_id() : 0;
        m.start_time = miningHandler.start_time();
        m.mining_level = 1 + player.GetMineExp() / 100;
        return m;
    };

    const auto make_blacksmith_view = [&]() -> mud::view::BlacksmithView {
        mud::view::BlacksmithView v;
        v.gold = gold;
        for (const auto id : {mud::tool::ToolId::Hoe, mud::tool::ToolId::Rod, mud::tool::ToolId::Pickaxe})
        {
            mud::view::ToolRepairView tr;
            tr.name = tools.name(id);
            tr.durability = tools.durability(id);
            tr.max_durability = tools.max_durability(id);
            tr.broken = tools.is_broken(id);
            tr.repair_gold = tools.gold_repair_cost(id);
            tr.repair_ore = tools.repair_ore(id);
            tr.repair_ore_needed = tools.repair_ore_count(id);
            tr.repair_ore_held = player.GetBag().CountObject(tools.repair_ore(id));
            v.tools.push_back(tr);
        }
        return v;
    };

    // 输入格式提示：根据玩家当前位置给出可用指令的行动名称，供每次输入前展示。
    // 交互模式下用户只需输入行动名称（如 farm.sow），系统会逐个提示参数。
    const auto input_hint = [&]() -> std::string {
        const char* place = nullptr;
        std::string cmds;
        switch (player.GetPosition())
        {
            case AtHome:
                place = "小屋（你的起点）";
                cmds = "  player.status —— 查看角色状态\n"
                       "  farm.status —— 查看农田概况\n"
                       "  fish.status —— 查看鱼池概况\n"
                       "  move.<up/down/left/right> —— 向对应方向移动";
                break;
            case AtFarmland:
                place = "农田";
                cmds = "  farm.status —— 查看农田\n"
                       "  farm.sow —— 播种\n"
                       "  farm.water —— 浇水\n"
                       "  farm.fertilize —— 施肥\n"
                       "  farm.harvest —— 收割\n"
                       "  move.<方向> —— 离开农田";
                break;
            case AtCoast:
                place = "海岸";
                cmds = "  fish.status —— 查看鱼池\n"
                       "  fish.tick —— 抛竿垂钓（每轮 3-6 秒，按 q 中止）\n"
                       "  move.<方向> —— 离开海岸";
                break;
            case AtMine:
                place = "矿洞";
                cmds = "  mine.status —— 查看采矿状态\n"
                       "  mine.start —— 开始采矿（每轮 3-6 秒，按 q 中止）\n"
                       "  mine.stop —— 手动结束采矿\n"
                       "  move.<方向> —— 离开矿洞";
                break;
            case AtTown:
                place = "小镇（集市所在地）";
                cmds = "  market.status —— 查看集市行情\n"
                       "  market.buy —— 购物\n"
                       "  market.sell —— 出售背包物品\n"
                       "  blacksmith.status —— 查看铁匠铺修复信息\n"
                       "  blacksmith.repair —— 修复工具\n"
                       "  move.<方向> —— 离开小镇";
                break;
        }
        return "【" + std::string(place) + "】当前可用指令：\n" + cmds
             + "\n通用指令：time.now 时间 | weather.now 天气 | tools.status 工具耐久"
               " | save 存档 | load 读档 | help 帮助 | quit 退出";
    };

    // 每次输入前先打印当前位置的输入格式提示，再输出命令提示符。
    const auto show_prompt = [&]() {
        mud::view::print_input_hint(outRenderer, input_hint());
        terminal.print_prompt();
    };

    // 通用输出：当前时刻 + 天气
    const auto render_now = [&]() {
        terminal.render_now(make_time_view(), make_weather_view());
    };

    // 单行反馈文本：委托 MessagePanel 展示。
    const auto msg = [&](const std::string& text) {
        terminal.render_message(mud::view::MessageLine{text});
    };

// 前台等待循环：随机等待 kActionWaitMinMs~kActionWaitMaxMs 毫秒，
    // 期间逐帧轮询键盘——按下 q/Q 立即返回 true（用户请求退出），
    // 等待自然结束返回 false。返回 true 时已排空残留输入，避免污染 REPL。
    const auto wait_action = [&]() -> bool {
        const int wait_ms =
            kActionWaitMinMs + std::rand() % (kActionWaitMaxMs - kActionWaitMinMs + 1);
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(wait_ms);
        while (std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            while (_kbhit())
            {
                const int ch = _getch();
                if (ch == 'q' || ch == 'Q')
                {
                    // 排空同一行残留的回车/换行，避免被下一轮 getline 当成输入
                    while (_kbhit()) (void)_getch();
                    return true;
                }
            }
        }
        return false;
    };

    // 采矿上下文生成：由玩家采矿经验推导等级，由矿镐等级推导速度与间隔
    const auto build_mine_ctx = [&]() -> mining::MiningContext {
        mining::MiningContext mc;
        mc.mining_level = 1 + static_cast<std::size_t>(player.GetMineExp() / 100);
        mc.has_torch = true;      // 演示：默认持有照明
        mc.has_lantern = true;
        const int pick = tools.level(mud::tool::ToolId::Pickaxe);
        switch (pick) {
            case 5: mc.tool.mining_speed = mining::MiningSpeed::Iron;    break;
            case 4: mc.tool.mining_speed = mining::MiningSpeed::Silver;  break;
            case 3: mc.tool.mining_speed = mining::MiningSpeed::Gold;    break;
            case 2: mc.tool.mining_speed = mining::MiningSpeed::Crystal; break;
            default: mc.tool.mining_speed = mining::MiningSpeed::Core;   break;
        }
        mc.tool.interval = std::max<std::int64_t>(1, 7 - pick); // 矿镐等级越高，单次采矿间隔越短
        return mc;
    };

    // 采矿产出入库：按矿石数据复制新 Object 进背包，并累计采矿经验；
    // 反馈文本统一收集后交由 MiningPanel 渲染。
    const auto grant_mining = [&](const std::vector<mining::MiningResult>& results, bool render = true) {
        std::vector<mud::view::MiningEventView> events;
        for (const auto& r : results) {
            const std::string name = oreTable.get_ore_name(r.ore_id);
            const std::string usage = oreTable.get_ore_usage(r.ore_id);
            const auto price = oreTable.get_ore_price(r.ore_id);
            for (std::size_t i = 0; i < r.quantity; ++i)
                player.GetBag().AddObject(new Object(name, usage, 0,
                     static_cast<int>(price), 0));
            player.SetMineExp(player.GetMineExp() + static_cast<int>(r.experience));
            events.push_back({name, r.quantity, r.experience});
        }
        if (render && !events.empty())
            terminal.render_mining_produce(events);
    };

    // 世界推进（后台线程每现实秒调用）：经 TimeService::update() 按 time_scale
    // 推进游戏时间，随后对实际跨过的每个游戏分钟驱动天气/作物/钓鱼，
    // 跨天刷新集市；若在采矿则按到期进度统一结算产出。
    // 注意：time_scale 可非整分钟（如 0.5），整分钟未凑满时跳过本次推进。
    const auto tick_world = [&]() {
        const auto before = timeService.now();
        timeService.update();
        const std::int64_t before_total = before.total_minutes();
        const std::int64_t now_total = timeService.now().total_minutes();
        if (now_total <= before_total) return; // 亚分钟未凑满整分钟，无需推进

        std::int64_t last_day = before_total / 1440;
        auto cursor = before;
        for (std::int64_t t = before_total; t < now_total; ++t) {
            cursor.advance(1);
            weather.update();
            farming.tick(cursor);
            fishing.tick(cursor);
            const std::int64_t day = cursor.total_minutes() / 1440;
            if (day != last_day) {
                market.onNewDay(cursor);
                last_day = day;
            }
        }
        if (miningHandler.is_mining()) {
            const auto results = miningHandler.poll(build_mine_ctx());
            grant_mining(results, false); // 后台静默入包，不打断正在键入的命令
        }
    };

    // ================= 命令处理器注册 =================
    // 采矿
    // 前置校验：位置 / 层条件；然后在前台做"等待 3-6 秒出一次结果"的循环，
    // 期间不可操作，按 q 键退出并回到指令模式（不再依赖后台按游戏分钟结算）。
    connector.bind("mine.start", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (player.GetPosition() != AtMine) {
            msg("你不在矿区。");
            return HandlerResult::Failed;
        }
        const std::size_t layer = static_cast<std::size_t>(opt_int(cmd, "layer", 0));
        const auto mc = build_mine_ctx();
        if (!miningController.can_enter(layer, mc)) {
            msg("层条件不满足（等级或照明不足）。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Mining);
        msg("开始采矿（层 " + std::to_string(layer) + "）：每轮等待 3-6 秒后出矿，"
            "期间不可操作，按 q 退出。");
        int cycles = 0;
        bool stop = false;
        while (true) {
            if (wait_action()) {          // 等待 3-6 秒（按 q 中止）
                stop = true;
                break;
            }
            auto result = miningController.produce_once(layer, mc);
            if (!result) {
                msg("矿镐损坏或矿洞塌方，采矿中断。");
                break;
            }
            ++cycles;
            std::vector<mining::MiningResult> batch;
            batch.push_back(std::move(*result));
            grant_mining(batch);
        }
        player.SetState(StateCode::Waiting);
        msg(stop ? "采矿结束（手动退出），共出矿 " + std::to_string(cycles) + " 次。"
                 : "采矿结束，共出矿 " + std::to_string(cycles) + " 次。");
        return HandlerResult::Ok;
    });

    connector.bind("mine.stop", [&](const mud::cmd::Command&, const HandlerContext&) {
        if (!miningHandler.is_mining()) {
            msg("当前并未在采矿。");
            return HandlerResult::Failed;
        }
        const auto results = miningHandler.stop(build_mine_ctx());
        player.SetState(StateCode::Waiting);
        grant_mining(results);
        msg("采矿结束。");
        return HandlerResult::Ok;
    });

    connector.bind("mine.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        terminal.render_mining_status(make_mining_view());
        return HandlerResult::Ok;
    });

    // 时间
    connector.bind("time.now", [&](const mud::cmd::Command&, const HandlerContext&) {
        render_now();
        return HandlerResult::Ok;
    });

    connector.bind("time.scale", [&](const mud::cmd::Command& cmd, const HandlerContext& ctx2) {
        const double factor = std::stod(cmd.options.at("factor"));
        ctx2.time.set_time_scale(factor);
        terminal.render_time_scale(factor);
        return HandlerResult::Ok;
    });

    // 玩家状态 / 移动
    connector.bind("player.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        terminal.render_status(make_player_status());
        return HandlerResult::Ok;
    });

    const auto bind_move = [&](const std::string& verb, bool (Move::*fn)()) {
        connector.bind(verb, [&, fn](const mud::cmd::Command&, const HandlerContext&) {
            if (!weather.can_go_outside()) {
                msg(weather.weather_name() + "天不宜外出。");
                return HandlerResult::Failed;
            }
            player.SetState(StateCode::Moving);
            if ((move.*fn)()) {
                msg("移动成功。");
                terminal.render_map(player.GetPosition());
            } else {
                msg("这个方向走不通。");
            }
            player.SetState(StateCode::Waiting); // 一次性动作完成后复位，避免状态粘滞
            return HandlerResult::Ok;
        });
    };
    bind_move("move.up",    &Move::GoUp);
    bind_move("move.down",  &Move::GoDown);
    bind_move("move.left",  &Move::GoLeft);
    bind_move("move.right", &Move::GoRight);

    // 农田
    connector.bind("farm.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        terminal.render_farm(make_farm_view());
        return HandlerResult::Ok;
    });

    const auto valid_plot = [&](std::size_t idx) {
        return idx < farming.farmSize();
    };

    connector.bind("farm.sow", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (player.GetPosition() != AtFarmland) {
            msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "plot", 0));
        const std::string crop_key = cmd.options.count("crop") ? cmd.options.at("crop") : "";
        const auto it = kSeeds.find(crop_key);
        if (it == kSeeds.end()) {
            msg("没有这种作物：" + crop_key);
            return HandlerResult::BadArgument;
        }
        const Crop* crop = it->second;
        if (crop->getUnlockLevel() > 1 + player.GetFarmingExp() / 100) {
            msg("作物未解锁（需种植经验 ≥ " + std::to_string(crop->getUnlockLevel() * 100) + "）。");
            return HandlerResult::Failed;
        }
        if (!valid_plot(idx) || !farming.sow(idx, it->second)) {
            msg("播种失败（地块占用或索引越界）。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Seeding);
        msg("已播种 " + kCropNames.at(crop) + "。");
        player.SetState(StateCode::Waiting); // 播种为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.water", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (player.GetPosition() != AtFarmland) {
            msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "plot", 0));
        if (!valid_plot(idx) || !farming.water(idx)) {
            msg("浇水失败（地块无作物或索引越界）。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Watering);
        weather.mark_watered(); // 手动浇水计入"今日已浇水"
        msg("已浇水。");
        player.SetState(StateCode::Waiting); // 浇水为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.fertilize", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (player.GetPosition() != AtFarmland) {
            msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "plot", 0));
        const std::string type = cmd.options.count("type") ? cmd.options.at("type") : "";
        Fertilizer* fert = nullptr;
        if (type == "normal") fert = &normalFert;
        else if (type == "advanced") fert = &advancedFert;
        if (fert == nullptr) {
            msg("未知肥料类型：" + type + "（normal/advanced）");
            return HandlerResult::BadArgument;
        }
        if (!valid_plot(idx) || !farming.fertilize(idx, fert)) {
            msg("施肥失败（地块无作物或索引越界）。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Fertilizing);
        msg("施肥完成。");
        player.SetState(StateCode::Waiting); // 施肥为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.harvest", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (player.GetPosition() != AtFarmland) {
            msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "plot", 0));
        if (!valid_plot(idx)) {
            msg("地块索引越界。");
            return HandlerResult::BadArgument;
        }
        auto& fl = farm.getFarmland(idx);
        const Crop* crop = fl.getCrop();
        const int yield = farming.harvest(idx);
        if (yield <= 0) {
            msg("尚无成熟作物。");
            return HandlerResult::Failed;
        }
        const std::string cname = kCropNames.at(crop);
        const int sell = crop->GetSellingPrice();
        const int buy = crop->GetBuyingPrice();
        for (int i = 0; i < yield; ++i)
            player.GetBag().AddObject(new Object(cname + "（收获）", "农田收获", 0, sell, buy));
        player.SetFarmingExp(player.GetFarmingExp() + crop->getFarmExp());
        msg("收获 " + cname + " x" + std::to_string(yield)
            + "（+" + std::to_string(crop->getFarmExp()) + " 种植经验）。");
        return HandlerResult::Ok;
    });

    // 钓鱼
    connector.bind("fish.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        terminal.render_fishing(make_fishing_view());
        return HandlerResult::Ok;
    });

    connector.bind("fish.tick", [&](const mud::cmd::Command&, const HandlerContext&) {
        if (player.GetPosition() != AtCoast) {
            msg("你不在海边。");
            return HandlerResult::Failed;
        }
        if (!weather.can_fish()) {
            msg(weather.weather_name() + "天不能钓鱼。");
            return HandlerResult::Failed;
        }
        if (player.GetSatiety() <= 0) {
            msg("体力不足，无法钓鱼。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Fishing);
        int caught = 0;
        msg("开始钓鱼：每轮等待 3-6 秒后出结果，期间不可操作，按 q 退出。");
        bool stop = false;
        while (true) {
            if (wait_action()) {          // 等待 3-6 秒（按 q 中止）
                stop = true;
                break;
            }
            // 每轮垂钓消耗体力（饱食度）
            if (player.GetSatiety() <= 0) {
                msg("体力耗尽，钓鱼停止。");
                break;
            }
            player.SetSatiety(player.GetSatiety() - kFishingSatietyCost);
            Fish* f = fishing.tickFish();
            if (f == nullptr) {
                msg("  这一轮没有钓到鱼。");
                continue;
            }
            const std::string fname = kFishNames.at(f);
            player.GetBag().AddObject(new Object(fname, "刚钓上来的鱼", 0,
                f->GetSellingPrice(), f->GetBuyingPrice()));
            player.SetFishExp(player.GetFishExp() + f->getFishExp());
            msg("  钓到 " + fname + "！");
            ++caught;
        }
        player.SetState(StateCode::Waiting);
        msg(stop ? "钓鱼结束（手动退出），共钓到 " + std::to_string(caught) + " 条鱼。"
                 : "钓鱼结束，共钓到 " + std::to_string(caught) + " 条鱼。");
        return HandlerResult::Ok;
    });

    // 天气
    connector.bind("weather.now", [&](const mud::cmd::Command&, const HandlerContext&) {
        terminal.render_weather(make_weather_view());
        return HandlerResult::Ok;
    });

    // 集市
    connector.bind("market.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        terminal.render_market(make_market_view());
        return HandlerResult::Ok;
    });

    connector.bind("market.buy", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (player.GetPosition() != AtTown) {
            msg("你不在城镇。");
            return HandlerResult::Failed;
        }
        const std::string shop_id = cmd.options.count("shop") ? cmd.options.at("shop") : "";
        const std::string item_name = cmd.options.count("item") ? cmd.options.at("item") : "";
        const auto count = static_cast<std::size_t>(opt_int(cmd, "count", 1));
        Shop* shop = market.findShop(shop_id);
        if (shop == nullptr) {
            msg("没有这家商店：" + shop_id);
            return HandlerResult::BadArgument;
        }
        Object* item = nullptr;
        const auto all_digits = [](const std::string& s) {
            if (s.empty()) return false;
            for (char c : s) if (c < '0' || c > '9') return false;
            return true;
        };
        if (all_digits(item_name)) {
            const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "item", 0));
            if (idx < shop->itemCount()) item = shop->getItem(idx).getItem();
        } else {
            for (std::size_t i = 0; i < shop->itemCount(); ++i) {
                Object* cand = shop->getItem(i).getItem();
                if (cand != nullptr && cand->GetName() == item_name) { item = cand; break; }
            }
        }
        if (item == nullptr) {
            msg("商店没有这种商品：" + item_name);
            return HandlerResult::BadArgument;
        }
        const int price = market.getBuyPrice(shop_id, item);
        if (!market.buy(shop_id, item, static_cast<int>(count), gold)) {
            msg("购买失败（金币不足或商品缺货）。");
            return HandlerResult::Failed;
        }
        for (std::size_t i = 0; i < count; ++i)
            player.GetBag().AddObject(new Object(item->GetName(), item->GetDescription(),
                item->GetHealth(), item->GetSellingPrice(), item->GetBuyingPrice()));
        msg("购入 " + item->GetName() + " x" + std::to_string(count)
            + "（花费 " + std::to_string(price * count) + "）。");
        return HandlerResult::Ok;
    });

    connector.bind("market.sell", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (player.GetPosition() != AtTown) {
            msg("你不在城镇。");
            return HandlerResult::Failed;
        }
        const std::string raw_item = cmd.options.count("item") ? cmd.options.at("item") : "";
        const auto count = static_cast<std::size_t>(opt_int(cmd, "count", 1));
        const auto all_digits = [](const std::string& s) {
            if (s.empty()) return false;
            for (char c : s) if (c < '0' || c > '9') return false;
            return true;
        };
        // 去重后的同名词（每个堆叠一项）
        const auto& all_names = player.GetBag().GetAllObjectName();
        std::string item_name;
        if (all_digits(raw_item)) {
            const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "item", 0));
            if (idx >= all_names.size()) {
                msg("背包里没有序号 " + std::to_string(idx) + " 的物品。");
                return HandlerResult::BadArgument;
            }
            item_name = all_names[idx];
        } else {
            item_name = raw_item;
            bool found = false;
            for (const auto& n : all_names) if (n == item_name) { found = true; break; }
            if (!found) {
                msg("背包里没有：" + item_name);
                return HandlerResult::BadArgument;
            }
        }
        // 堆叠总数量
        const int have = player.GetBag().CountObject(item_name);
        const auto sell_count = static_cast<std::size_t>(std::min<long long>(
            static_cast<long long>(count), static_cast<long long>(have)));
        if (sell_count == 0) {
            msg("背包里没有：" + item_name);
            return HandlerResult::BadArgument;
        }
        Object* item = nullptr;
        for (auto* obj : player.GetBag().GetObjects()) {
            if (obj->GetName() == item_name) { item = obj; break; }
        }
        if (item == nullptr) {
            msg("背包里没有：" + item_name);
            return HandlerResult::BadArgument;
        }
        const int gained = market.sell(item, static_cast<int>(sell_count), gold);
        player.GetBag().RemoveObject(item_name, static_cast<int>(sell_count));
        msg("出售" + item_name + " x" + std::to_string(sell_count) + "，获得金币 "
            + std::to_string(gained) + "。");
        return HandlerResult::Ok;
    });

    // 工具
    connector.bind("tools.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        terminal.render_tools(make_tools_view());
        return HandlerResult::Ok;
    });

    // 铁匠铺（集市内，工具修复服务）
    connector.bind("blacksmith.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        if (player.GetPosition() != AtTown) {
            msg("你不在城镇，先去小镇集市看看。");
            return HandlerResult::Failed;
        }
        terminal.render_blacksmith(make_blacksmith_view());
        return HandlerResult::Ok;
    });

    connector.bind("blacksmith.repair", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (player.GetPosition() != AtTown) {
            msg("你不在城镇，无法前往铁匠铺。");
            return HandlerResult::Failed;
        }
        const std::string tool_key = cmd.options.count("tool") ? cmd.options.at("tool") : "";
        const std::string method = cmd.options.count("method") ? cmd.options.at("method") : "";
        mud::tool::ToolId id;
        if (tool_key == "hoe")             id = mud::tool::ToolId::Hoe;
        else if (tool_key == "rod")        id = mud::tool::ToolId::Rod;
        else if (tool_key == "pickaxe")    id = mud::tool::ToolId::Pickaxe;
        else {
            msg("未知工具：" + tool_key + "（可用 hoe/rod/pickaxe）");
            return HandlerResult::BadArgument;
        }
        if (method != "gold" && method != "ore") {
            msg("未知修复方式：" + method + "（可用 ore/gold）");
            return HandlerResult::BadArgument;
        }
        if (tools.is_full(id)) {
            msg(tools.name(id) + " 完好无损，无需修复。");
            return HandlerResult::Ok;
        }
        const int gold_cost = tools.gold_repair_cost(id);
        const std::string ore = tools.repair_ore(id);
        const int ore_needed = tools.repair_ore_count(id);

        if (method == "ore") {
            const int ore_cost = tools.ore_repair_cost(id);
            const int ore_held = player.GetBag().CountObject(ore);
            if (ore_held < ore_needed) {
                msg("矿石不足：" + ore + " 需要 x" + std::to_string(ore_needed)
                    + "，当前持有 " + std::to_string(ore_held) + "。");
                return HandlerResult::Failed;
            }
            if (gold < ore_cost) {
                msg("金币不足，矿石修复还需 " + std::to_string(ore_cost) + " 金币（当前 "
                    + std::to_string(gold) + "）。");
                return HandlerResult::Failed;
            }
            player.GetBag().RemoveObject(ore, ore_needed);
            gold -= ore_cost;
            player.SetState(StateCode::Repairing);
            tools.repair_full(id);
            player.SetState(StateCode::Waiting);
            msg("铁匠用 " + ore + " x" + std::to_string(ore_needed) + " + 金币 "
                + std::to_string(ore_cost) + " 修好了" + tools.name(id) + "。");
            return HandlerResult::Ok;
        }
        // 金币修复
        if (gold < gold_cost) {
            msg("金币不足：修复" + tools.name(id) + " 需要 " + std::to_string(gold_cost)
                + " 金币（当前 " + std::to_string(gold) + "）。");
            return HandlerResult::Failed;
        }
        gold -= gold_cost;
        player.SetState(StateCode::Repairing);
        tools.repair_full(id);
        player.SetState(StateCode::Waiting);
        msg("铁匠花费 " + std::to_string(gold_cost) + " 金币修好了" + tools.name(id) + "。");
        return HandlerResult::Ok;
    });

    // 存档 / 读档
    connector.bind("save", [&](const mud::cmd::Command&, const HandlerContext&) {
        PlayerSerializer serializer;
        if (serializer.Save(kSaveFileName, player, game, gold, tools,
                            timeService.session_total())) {
            msg(std::string("存档成功：") + kSaveFileName);
        } else {
            msg("存档失败（无法写入存档文件）。");
        }
        return HandlerResult::Ok;
    });

    connector.bind("load", [&](const mud::cmd::Command&, const HandlerContext&) {
        PlayerSerializer serializer;
        std::int64_t totalMinutes = -1;
        if (!serializer.Load(kSaveFileName, player, game, gold, tools, totalMinutes)) {
            msg(std::string("读档失败：没有找到存档 ") + kSaveFileName + "。");
            return HandlerResult::Failed;
        }
        if (totalMinutes >= 0) {
            mud::time::GameDateTime t;
            t.advance(totalMinutes); // 由存档总分钟数重建游戏内时钟
            timeService.set_time(t);
        }
        msg(std::string("读档成功，继续你的冒险吧。"));
        return HandlerResult::Ok;
    });

    // ================= 命令参数 Schema 注册（交互模式）=================
    // 定义每个命令需要的参数，供 ParameterCollector 逐个提示用户输入。
    // 动态提示内容在注册时从游戏状态捕获。

    const std::size_t farmSize = farming.farmSize();

    connector.register_schema("mine.start", {
        "开始采矿",
        {{"layer", "目标层(0-4)", true, ""}}
    });

    connector.register_schema("time.scale", {
        "设置时间倍率",
        {{"factor", "时间倍率(如 0.5/1/2/60)", true, ""}}
    });

    connector.register_schema("farm.sow", {
        "播种",
        {{"plot", "地块索引(0-" + std::to_string(farmSize - 1) + ")", true, ""},
         {"crop", "作物名(cabbage/carrot/tomato/pumpkin/lingzhi)", true, ""}}
    });

    connector.register_schema("farm.water", {
        "浇水",
        {{"plot", "地块索引(0-" + std::to_string(farmSize - 1) + ")", true, ""}}
    });

    connector.register_schema("farm.fertilize", {
        "施肥",
        {{"plot", "地块索引(0-" + std::to_string(farmSize - 1) + ")", true, ""},
         {"type", "肥料类型(normal/advanced)", true, ""}}
    });

    connector.register_schema("farm.harvest", {
        "收割",
        {{"plot", "地块索引(0-" + std::to_string(farmSize - 1) + ")", true, ""}}
    });

    connector.register_schema("market.buy", {
        "从商店购买",
        {{"shop", "商店ID(seed/grocery)", true, ""},
         {"item", "物品名", true, ""},
         {"count", "购买数量", false, "1"}}
    });

    connector.register_schema("market.sell", {
        "向集市出售背包物品",
        {{"item", "物品名或序号", true, ""},
         {"count", "出售数量", false, "1"}}
    });

    connector.register_schema("blacksmith.repair", {
        "在铁匠铺修复工具",
        {{"tool", "工具名(hoe/rod/pickaxe)", true, ""},
         {"method", "修复方式(ore/gold)", true, ""}}
    });

    // ================= 自动时间推进（真实时间后台） =================
    // worldMutex 串行化「后台推进」与「REPL 命令执行」：命令处理与逐分钟更新
    // 共享 farm/market/weather/player/mining 等状态，必须互斥。
    std::mutex worldMutex;

    // 后台线程每现实秒调 timeService.update()（按 time_scale 推进游戏时间）
    // 并驱动世界推进；quit 时经 jthread 请求停止并自动 join。
    std::jthread timeThread([&](std::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (st.stop_requested()) break;
            std::lock_guard<std::mutex> lock(worldMutex);
            tick_world();
        }
    });

    // ================= 主循环（交互模式）=================
    // 用户只需输入行动名称（如 farm.sow），系统自动逐个提示参数。
    terminal.render_message(mud::view::MessageLine{
        "欢迎来到 胡萝卜山谷 MUD！输入行动名称即可，系统会逐个提示所需参数。\n"
        "输入 help 查看全部指令，quit 退出。"});
    render_now();
    terminal.render_map(player.GetPosition());
    show_prompt();

    std::string line;
    while (std::getline(std::cin, line)) {
        const std::string verb = parser.parse_verb_only(line);
        if (verb.empty()) {
            show_prompt();
            continue;
        }
        if (verb == "help") {
            terminal.render_message(mud::view::MessageLine{
                "可用指令（输入行动名称后按提示填写参数）：\n"
                "  time.now —— 查看当前时间\n"
                "  time.scale —— 设置时间倍率\n"
                "  player.status —— 查看角色状态\n"
                "  move.up / move.down / move.left / move.right —— 移动\n"
                "  farm.status —— 查看农田\n"
                "  farm.sow —— 播种（需地块索引和作物名）\n"
                "  farm.water —— 浇水（需地块索引）\n"
                "  farm.fertilize —— 施肥（需地块索引和肥料类型）\n"
                "  farm.harvest —— 收割（需地块索引）\n"
                "  fish.status —— 查看鱼池\n"
                "  fish.tick —— 钓鱼\n"
                "  weather.now —— 查看天气\n"
                "  mine.status —— 查看采矿状态\n"
                "  mine.start —— 开始采矿（需层号）\n"
                "  mine.stop —— 停止采矿\n"
                "  market.status —— 查看集市\n"
                "  market.buy —— 购物（需商店/物品/数量）\n"
                "  market.sell —— 出售（需物品/数量）\n"
                "  tools.status —— 查看工具耐久\n"
                "  blacksmith.status —— 查看铁匠铺\n"
                "  blacksmith.repair —— 修复工具（需工具名/方式）\n"
                "  save —— 存档  |  load —— 读档  |  quit —— 退出"});
        } else if (verb == "quit") {
            terminal.render_message(mud::view::MessageLine{"再见！"});
            break;
        } else {
            mud::cmd::Command cmd;
            cmd.verb = verb;
            cmd.raw = line;
            HandlerResult result;
            {
                std::lock_guard<std::mutex> lock(worldMutex);
                result = connector.dispatch(cmd, ctx);
            }
            if (result == HandlerResult::UnknownCommand)
                terminal.render_message(mud::view::MessageLine{
                    "未知命令：" + verb + "（输入 help 查看）"});
        }
        show_prompt();
    }

    timeThread.request_stop();
    game.endSession();
    return 0;
}