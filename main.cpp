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
#include <random>
#include <thread>

#include <windows.h>

#include "TuiState.h"
#include "TuiRenderer.h"
#include "GameTui.h"
#include "MusicPlayer.h"

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
#include "Pumpkin.h"
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
#include "MarketController.h"
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
    Cabbage cabbage;
    Carrot carrot;
    Tomato tomato;
    Pumpkin pumpkin;
    Lingzhi lingzhi;
    NormalFertilizer normalFert;
    AdvancedFertilizer advancedFert;

    const std::map<std::string, Crop*> kSeeds = {
        {"cabbage", &cabbage}, {"carrot", &carrot}, {"tomato", &tomato},
        {"pumpkin", &pumpkin}, {"lingzhi", &lingzhi}
    };
    const std::map<const Crop*, std::string> kCropNames = {
        {&cabbage, "小白菜"}, {&carrot, "胡萝卜"}, {&tomato, "番茄"},
        {&pumpkin, "南瓜"}, {&lingzhi, "灵芝"}
    };

    // ---- 鱼池（堆栈持有，钓到即复制入背包，绝不共享指针）----
    Crucian crucian;
    GrassCarp grassCarp;
    Perch perch;
    RainbowTrout rainbowTrout;
    KingCrab kingCrab;
    const std::vector<Fish*> kFishPool = {&crucian, &grassCarp, &perch, &rainbowTrout, &kingCrab};
    const std::map<const Fish*, std::string> kFishNames = {
        {&crucian, "小鲫鱼"}, {&grassCarp, "草鱼"}, {&perch, "鲈鱼"},
        {&rainbowTrout, "虹鳟鱼"}, {&kingCrab, "帝王蟹"}
    };
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
    long long gold = 200;
    // 集市交易控制器：承载 market.buy/sell 完整业务流程（处理器下沉）
    MarketController marketCtl(market, player.GetBag());

    // ---- 天气 ----
    WeatherController weather(timeService, farm);

    // ---- 工具 / 采矿 ----
    mud::tool::ToolController tools;
    Ore::OreData oreData{}; // 控制器仅持引用、无需数据文件内容
    Ore oreTable; // 供玩家反馈名称/价格/用途
    mud::event::EventSystem miningEvents;
    MiningController miningController(oreData, timeService, &miningEvents, &tools);
    MiningHandler miningHandler(miningController);

    // ---- 视图（View 层唯一输出出口）----
    // 改用 FTXUI TUI：输出经 TuiRenderer 重定向到界面日志区。
    mud::tui::TuiState tuiState;
    mud::tui::TuiRenderer outRenderer(tuiState);
    mud::view::TerminalView terminal(outRenderer);

    // ---- 解析 / 派发 ----
    InputParser parser;
    Connector connector;
    // 交互式参数收集改由 TUI 状态机驱动，故不再注册阻塞式 ParameterCollector。
    HandlerContext ctx{timeService, miningHandler};

    // ================= DTO 装配（组合根 → View 的唯一数据通道）=================
    const auto make_time_view = [&]() -> mud::view::TimeView
    {
        const auto now = timeService.now();
        mud::view::TimeView t;
        t.year = now.year;
        t.month = now.month;
        t.day = now.day;
        t.hour = now.hour;
        t.minute = now.minute;
        t.time_scale = timeService.time_scale();
        return t;
    };

    const auto make_weather_view = [&]() -> mud::view::WeatherView
    {
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

    const auto make_player_status = [&]() -> PlayerStatus
    {
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

    const auto make_farm_view = [&]() -> mud::view::FarmView
    {
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
                                              ? kCropNames.at(fl.getCrop())
                                              : "未知作物";
                p.crop_name = cname;
                const int growthCycle = fl.getCrop()->getGrowthCycle();
                p.growth_stage = fl.getGrowthStage() > growthCycle
                                     ? growthCycle
                                     : fl.getGrowthStage(); // 防御性钳制
                p.growth_max = growthCycle;
                p.watered = fl.isWatered();
            }
            f.plots.push_back(p);
        }
        return f;
    };

    const auto make_fishing_view = [&]() -> mud::view::FishingView
    {
        mud::view::FishingView f;
        for (const Fish* fish : kFishPool)
            f.pool.push_back({kFishNames.at(fish), fish->getProbability()});
        f.can_fish = weather.can_fish();
        return f;
    };

    const auto make_market_view = [&]() -> mud::view::MarketView
    {
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

    const auto make_tools_view = [&]() -> mud::view::ToolsView
    {
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

    const auto make_mining_view = [&]() -> mud::view::MiningView
    {
        mud::view::MiningView m;
        m.is_mining = miningHandler.is_mining();
        m.layer = miningHandler.layer_id() ? *miningHandler.layer_id() : 0;
        m.start_time = miningHandler.start_time();
        m.mining_level = 1 + player.GetMineExp() / 100;
        return m;
    };

    const auto make_blacksmith_view = [&]() -> mud::view::BlacksmithView
    {
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
    const auto input_hint = [&]() -> std::string
    {
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

    // 每次输入前先刷新输入法提示（TUI 在输入框上方展示）。
    // 该提示仅在空闲（无输入、无补全、无问题）时显示。
    const auto refresh_prompt = [&]()
    {
        tuiState.input_hint = input_hint();
    };

    // 通用输出：当前时刻 + 天气
    const auto render_now = [&]()
    {
        terminal.render_now(make_time_view(), make_weather_view());
    };

    // 单行反馈文本：委托 MessagePanel 展示（进入 TUI 日志区）。
    const auto msg = [&](const std::string& text)
    {
        terminal.render_message(mud::view::MessageLine{text});
    }; // 采矿上下文生成：由玩家采矿经验推导等级，由矿镐等级推导速度与间隔
    const auto build_mine_ctx = [&]() -> mining::MiningContext
    {
        mining::MiningContext mc;
        mc.mining_level = 1 + static_cast<std::size_t>(player.GetMineExp() / 100);
        mc.has_torch = true; // 演示：默认持有照明
        mc.has_lantern = true;
        const int pick = tools.level(mud::tool::ToolId::Pickaxe);
        switch (pick)
        {
        case 5: mc.tool.mining_speed = mining::MiningSpeed::Iron;
            break;
        case 4: mc.tool.mining_speed = mining::MiningSpeed::Silver;
            break;
        case 3: mc.tool.mining_speed = mining::MiningSpeed::Gold;
            break;
        case 2: mc.tool.mining_speed = mining::MiningSpeed::Crystal;
            break;
        default: mc.tool.mining_speed = mining::MiningSpeed::Core;
            break;
        }
        mc.tool.interval = std::max<std::int64_t>(1, 7 - pick); // 矿镐等级越高，单次采矿间隔越短
        return mc;
    };

    // 采矿产出入库：按矿石数据复制新 Object 进背包，并累计采矿经验；
    // 反馈文本统一收集后交由 MiningPanel 渲染。
    const auto grant_mining = [&](const std::vector<mining::MiningResult>& results, bool render = true)
    {
        std::vector<mud::view::MiningEventView> events;
        for (const auto& r : results)
        {
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
    const auto tick_world = [&]()
    {
        const auto before = timeService.now();
        timeService.update();
        const std::int64_t before_total = before.total_minutes();
        const std::int64_t now_total = timeService.now().total_minutes();
        if (now_total <= before_total) return; // 亚分钟未凑满整分钟，无需推进

        std::int64_t last_day = before_total / 1440;
        auto cursor = before;
        for (std::int64_t t = before_total; t < now_total; ++t)
        {
            cursor.advance(1);
            weather.update();
            farming.tick(cursor);
            fishing.tick(cursor);
            const std::int64_t day = cursor.total_minutes() / 1440;
            if (day != last_day)
            {
                market.onNewDay(cursor);
                last_day = day;
            }
        }
        if (miningHandler.is_mining())
        {
            const auto results = miningHandler.poll(build_mine_ctx());
            grant_mining(results, false); // 后台静默入包，不打断正在键入的命令
        }
    };

    // ================= 命令处理器注册 =================
    // 采矿
    // TUI 环境下改以 MiningHandler 会话 + 后台时间线程 (tick_world) 非阻塞推进：
    // "开始采矿" 仅建立会话并提示，产出由后台逐游戏分钟结算，输入 q 或 mine.stop 结束。
    connector.bind("mine.start", [&](const mud::cmd::Command& cmd, const HandlerContext&)
    {
        if (player.GetPosition() != AtMine)
        {
            msg("你不在矿区。");
            return HandlerResult::Failed;
        }
        if (miningHandler.is_mining())
        {
            msg("当前已在采矿中。");
            return HandlerResult::Ok;
        }
        const std::size_t layer = static_cast<std::size_t>(opt_int(cmd, "layer", 0));
        const auto mc = build_mine_ctx();
        if (!miningController.can_enter(layer, mc))
        {
            msg("层条件不满足（等级或照明不足）。");
            return HandlerResult::Failed;
        }
        if (!miningHandler.start(layer, mc))
        {
            msg("开始采矿失败。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Mining);
        msg("开始采矿（层 " + std::to_string(layer) + "）：产出随游戏时间自动结算，"
            "输入 q（结束动作）或 mine.stop 退出。");
        return HandlerResult::Ok;
    });

    connector.bind("mine.stop", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        if (!miningHandler.is_mining())
        {
            msg("当前并未在采矿。");
            return HandlerResult::Failed;
        }
        const auto results = miningHandler.stop(build_mine_ctx());
        player.SetState(StateCode::Waiting);
        grant_mining(results);
        msg("采矿结束。");
        return HandlerResult::Ok;
    });

    connector.bind("mine.status", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        terminal.render_mining_status(make_mining_view());
        return HandlerResult::Ok;
    });

    // 时间
    connector.bind("time.now", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        render_now();
        return HandlerResult::Ok;
    });

    connector.bind("time.scale", [&](const mud::cmd::Command& cmd, const HandlerContext& ctx2)
    {
        double factor;
        // DEF-102：数值校验下沉 cmdparser，非法倍率不再令 REPL 崩溃
        if (!mud::cmd::parse_double(cmd.options.at("factor"), factor))
        {
            msg("非法倍率：请输入数字（如 60、600）。");
            return HandlerResult::BadArgument;
        }
        ctx2.time.set_time_scale(factor);
        // 显示钳制后的实际倍率（DEF-103：钳制范围 [0,1e6]）
        terminal.render_time_scale(ctx2.time.time_scale());
        return HandlerResult::Ok;
    });

    // 玩家状态 / 移动
    connector.bind("player.status", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        terminal.render_status(make_player_status());
        return HandlerResult::Ok;
    });

    const auto bind_move = [&](const std::string& verb, bool (Move::*fn)())
    {
        connector.bind(verb, [&, fn](const mud::cmd::Command&, const HandlerContext&)
        {
            if (!weather.can_go_outside())
            {
                msg(weather.weather_name() + "天不宜外出。");
                return HandlerResult::Failed;
            }
            player.SetState(StateCode::Moving);
            if ((move.*fn)())
            {
                msg("移动成功。");
                terminal.render_map(player.GetPosition());
            }
            else
            {
                msg("这个方向走不通。");
            }
            player.SetState(StateCode::Waiting); // 一次性动作完成后复位，避免状态粘滞
            return HandlerResult::Ok;
        });
    };
    bind_move("move.up", &Move::GoUp);
    bind_move("move.down", &Move::GoDown);
    bind_move("move.left", &Move::GoLeft);
    bind_move("move.right", &Move::GoRight);

    // 农田
    connector.bind("farm.status", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        terminal.render_farm(make_farm_view());
        return HandlerResult::Ok;
    });

    const auto valid_plot = [&](std::size_t idx)
    {
        return idx < farming.farmSize();
    };

    connector.bind("farm.sow", [&](const mud::cmd::Command& cmd, const HandlerContext&)
    {
        if (player.GetPosition() != AtFarmland)
        {
            msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "plot", 0));
        const std::string crop_key = cmd.options.count("crop") ? cmd.options.at("crop") : "";
        const auto it = kSeeds.find(crop_key);
        if (it == kSeeds.end())
        {
            msg("没有这种作物：" + crop_key);
            return HandlerResult::BadArgument;
        }
        const Crop* crop = it->second;
        if (crop->getUnlockLevel() > 1 + player.GetFarmingExp() / 100)
        {
            msg("作物未解锁（需种植经验 ≥ " + std::to_string(crop->getUnlockLevel() * 100) + "）。");
            return HandlerResult::Failed;
        }
        if (!valid_plot(idx) || !farming.sow(idx, it->second))
        {
            msg("播种失败（地块占用或索引越界）。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Seeding);
        msg("已播种 " + kCropNames.at(crop) + "。");
        player.SetState(StateCode::Waiting); // 播种为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.water", [&](const mud::cmd::Command& cmd, const HandlerContext&)
    {
        if (player.GetPosition() != AtFarmland)
        {
            msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "plot", 0));
        if (!valid_plot(idx) || !farming.water(idx))
        {
            msg("浇水失败（地块无作物或索引越界）。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Watering);
        weather.mark_watered(); // 手动浇水计入"今日已浇水"
        msg("已浇水。");
        player.SetState(StateCode::Waiting); // 浇水为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.fertilize", [&](const mud::cmd::Command& cmd, const HandlerContext&)
    {
        if (player.GetPosition() != AtFarmland)
        {
            msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "plot", 0));
        const std::string type = cmd.options.count("type") ? cmd.options.at("type") : "";
        Fertilizer* fert = nullptr;
        if (type == "normal") fert = &normalFert;
        else if (type == "advanced") fert = &advancedFert;
        if (fert == nullptr)
        {
            msg("未知肥料类型：" + type + "（normal/advanced）");
            return HandlerResult::BadArgument;
        }
        if (!valid_plot(idx) || !farming.fertilize(idx, fert))
        {
            msg("施肥失败（地块无作物或索引越界）。");
            return HandlerResult::Failed;
        }
        player.SetState(StateCode::Fertilizing);
        msg("施肥完成。");
        player.SetState(StateCode::Waiting); // 施肥为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.harvest", [&](const mud::cmd::Command& cmd, const HandlerContext&)
    {
        if (player.GetPosition() != AtFarmland)
        {
            msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(opt_int(cmd, "plot", 0));
        if (!valid_plot(idx))
        {
            msg("地块索引越界。");
            return HandlerResult::BadArgument;
        }
        auto& fl = farm.getFarmland(idx);
        const Crop* crop = fl.getCrop();
        const int yield = farming.harvest(idx);
        if (yield <= 0)
        {
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
    connector.bind("fish.status", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        terminal.render_fishing(make_fishing_view());
        return HandlerResult::Ok;
    });

    // 钓鱼（TUI 非阻塞版）：启动后由 handle_action_tick 按 3-6 秒节奏推进，
    // 输入 q 或再次 fish.tick（动作进行中）可结束。可反复 start/stop。
    const auto end_fishing = [&](bool manual)
    {
        player.SetState(StateCode::Waiting);
        if (manual)
            msg("钓鱼结束（手动退出），共钓到 " + std::to_string(tuiState.action_cycles) + " 条鱼。");
        else
            msg("钓鱼结束，共钓到 " + std::to_string(tuiState.action_cycles) + " 条鱼。");
        tuiState.action_type = mud::tui::ActionType::None;
        tuiState.action_cycles = 0;
        tuiState.action_next_ms = 0;
    };

    connector.bind("fish.tick", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        if (tuiState.action_type == mud::tui::ActionType::Fish)
        {
            end_fishing(true);
            return HandlerResult::Ok;
        }
        if (player.GetPosition() != AtCoast)
        {
            msg("你不在海边。");
            return HandlerResult::Failed;
        }
        if (!weather.can_fish())
        {
            msg(weather.weather_name() + "天不能钓鱼。");
            return HandlerResult::Failed;
        }
        if (player.GetSatiety() <= 0)
        {
            msg("体力不足，无法钓鱼。");
            return HandlerResult::Failed;
        }
        const auto now = std::chrono::steady_clock::now();
        tuiState.action_type = mud::tui::ActionType::Fish;
        tuiState.action_cycles = 0;
        tuiState.action_next_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
            + kActionWaitMinMs + std::rand() % (kActionWaitMaxMs - kActionWaitMinMs + 1);
        player.SetState(StateCode::Fishing);
        msg("开始钓鱼：每 3-6 秒自动出结果，输入 q（结束动作）或 fish.tick 退出。");
        return HandlerResult::Ok;
    });

    // 由 1 秒后台节拍调用的动作推进：钓鱼按记录的时间点逐轮结算。
    const auto handle_action_tick = [&]()
    {
        if (tuiState.action_type != mud::tui::ActionType::Fish)
            return;
        const auto now = std::chrono::steady_clock::now();
        const std::int64_t now_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        if (now_ms < tuiState.action_next_ms)
            return;
        if (player.GetSatiety() <= 0)
        {
            msg("体力耗尽，钓鱼停止。");
            end_fishing(false);
            return;
        }
        player.SetSatiety(player.GetSatiety() - kFishingSatietyCost);
        Fish* f = fishing.tickFish();
        if (f == nullptr)
        {
            msg("  这一轮没有钓到鱼。");
        }
        else
        {
            const std::string fname = kFishNames.at(f);
            player.GetBag().AddObject(new Object(fname, "刚钓上来的鱼", 0,
                                                 f->GetSellingPrice(), f->GetBuyingPrice()));
            player.SetFishExp(player.GetFishExp() + f->getFishExp());
            msg("  钓到 " + fname + "！");
        }
        ++tuiState.action_cycles;
        tuiState.action_next_ms = now_ms + kActionWaitMinMs
            + std::rand() % (kActionWaitMaxMs - kActionWaitMinMs + 1);
    };

    // 天气
    connector.bind("weather.now", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        terminal.render_weather(make_weather_view());
        return HandlerResult::Ok;
    });

    // 集市
    connector.bind("market.status", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        terminal.render_market(make_market_view());
        return HandlerResult::Ok;
    });

    connector.bind("market.buy", [&](const mud::cmd::Command& cmd, const HandlerContext&)
    {
        if (player.GetPosition() != AtTown)
        {
            msg("你不在城镇。");
            return HandlerResult::Failed;
        }
        const std::string shop_id = cmd.options.count("shop") ? cmd.options.at("shop") : "";
        const std::string item_ref = cmd.options.count("item") ? cmd.options.at("item") : "";
        const int count = static_cast<int>(opt_int(cmd, "count", 1));
        // 业务流程（解析/扣款/入包/64 位总额）已下沉 MarketController
        const auto r = marketCtl.buy(shop_id, item_ref, count, gold);
        switch (r.status)
        {
        case MarketController::BuyResult::Status::NoSuchShop:
            msg("没有这家商店：" + shop_id);
            return HandlerResult::BadArgument;
        case MarketController::BuyResult::Status::NoSuchItem:
            msg("商店没有这种商品：" + item_ref);
            return HandlerResult::BadArgument;
        case MarketController::BuyResult::Status::BuyRejected:
            msg("购买失败（金币不足或商品缺货）。");
            return HandlerResult::Failed;
        case MarketController::BuyResult::Status::Ok:
            break;
        }
        msg("购入 " + r.item_name + " x" + std::to_string(r.bought)
            + "（花费 " + std::to_string(r.spent) + "）。");
        return HandlerResult::Ok;
    });

    connector.bind("market.sell", [&](const mud::cmd::Command& cmd, const HandlerContext&)
    {
        if (player.GetPosition() != AtTown)
        {
            msg("你不在城镇。");
            return HandlerResult::Failed;
        }
        const std::string item_ref = cmd.options.count("item") ? cmd.options.at("item") : "";
        const int count = static_cast<int>(opt_int(cmd, "count", 1));
        // 业务流程（名字/序号解析、数量钳制、售价守卫、移物加钱）已下沉 MarketController
        const auto r = marketCtl.sell(item_ref, count, gold);
        switch (r.status)
        {
        case MarketController::SellResult::Status::NoSuchItem:
            msg("背包里没有：" + item_ref);
            return HandlerResult::BadArgument;
        case MarketController::SellResult::Status::ZeroPrice:
            msg("出售失败：" + r.item_name + " 当前售价为 0。");
            return HandlerResult::Failed;
        case MarketController::SellResult::Status::Ok:
            break;
        }
        msg("出售" + r.item_name + " x" + std::to_string(r.sold) + "，获得金币 "
            + std::to_string(r.gained) + "。");
        return HandlerResult::Ok;
    });

    // 工具
    connector.bind("tools.status", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        terminal.render_tools(make_tools_view());
        return HandlerResult::Ok;
    });

    // 铁匠铺（集市内，工具修复服务）
    connector.bind("blacksmith.status", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        if (player.GetPosition() != AtTown)
        {
            msg("你不在城镇，先去小镇集市看看。");
            return HandlerResult::Failed;
        }
        terminal.render_blacksmith(make_blacksmith_view());
        return HandlerResult::Ok;
    });

    connector.bind("blacksmith.repair", [&](const mud::cmd::Command& cmd, const HandlerContext&)
    {
        if (player.GetPosition() != AtTown)
        {
            msg("你不在城镇，无法前往铁匠铺。");
            return HandlerResult::Failed;
        }
        const std::string tool_key = cmd.options.count("tool") ? cmd.options.at("tool") : "";
        const std::string method = cmd.options.count("method") ? cmd.options.at("method") : "";
        mud::tool::ToolId id;
        if (tool_key == "hoe") id = mud::tool::ToolId::Hoe;
        else if (tool_key == "rod") id = mud::tool::ToolId::Rod;
        else if (tool_key == "pickaxe") id = mud::tool::ToolId::Pickaxe;
        else
        {
            msg("未知工具：" + tool_key + "（可用 hoe/rod/pickaxe）");
            return HandlerResult::BadArgument;
        }
        if (method != "gold" && method != "ore")
        {
            msg("未知修复方式：" + method + "（可用 ore/gold）");
            return HandlerResult::BadArgument;
        }
        if (tools.is_full(id))
        {
            msg(tools.name(id) + " 完好无损，无需修复。");
            return HandlerResult::Ok;
        }
        const int gold_cost = tools.gold_repair_cost(id);
        const std::string ore = tools.repair_ore(id);
        const int ore_needed = tools.repair_ore_count(id);

        if (method == "ore")
        {
            const int ore_cost = tools.ore_repair_cost(id);
            const int ore_held = player.GetBag().CountObject(ore);
            if (ore_held < ore_needed)
            {
                msg("矿石不足：" + ore + " 需要 x" + std::to_string(ore_needed)
                    + "，当前持有 " + std::to_string(ore_held) + "。");
                return HandlerResult::Failed;
            }
            if (gold < ore_cost)
            {
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
        if (gold < gold_cost)
        {
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
    connector.bind("save", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        PlayerSerializer serializer;
        if (serializer.Save(kSaveFileName, player, game, gold, tools,
                            timeService.session_total()))
        {
            msg(std::string("存档成功：") + kSaveFileName);
        }
        else
        {
            msg("存档失败（无法写入存档文件）。");
        }
        return HandlerResult::Ok;
    });

    connector.bind("load", [&](const mud::cmd::Command&, const HandlerContext&)
    {
        PlayerSerializer serializer;
        std::int64_t totalMinutes = -1;
        // DEF-385：显式区分读档失败原因（文件不存在 / 损坏）
        if (serializer.Load(kSaveFileName, player, game, gold, tools, totalMinutes)
            != PlayerSerializer::LoadStatus::Ok)
        {
            msg(std::string("读档失败：没有找到存档 ") + kSaveFileName + "。");
            return HandlerResult::Failed;
        }
        if (totalMinutes >= 0)
        {
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
                                  {
                                      {"plot", "地块索引(0-" + std::to_string(farmSize - 1) + ")", true, ""},
                                      {"crop", "作物名(cabbage/carrot/tomato/pumpkin/lingzhi)", true, ""}
                                  }
                              });

    connector.register_schema("farm.water", {
                                  "浇水",
                                  {{"plot", "地块索引(0-" + std::to_string(farmSize - 1) + ")", true, ""}}
                              });

    connector.register_schema("farm.fertilize", {
                                  "施肥",
                                  {
                                      {"plot", "地块索引(0-" + std::to_string(farmSize - 1) + ")", true, ""},
                                      {"type", "肥料类型(normal/advanced)", true, ""}
                                  }
                              });

    connector.register_schema("farm.harvest", {
                                  "收割",
                                  {{"plot", "地块索引(0-" + std::to_string(farmSize - 1) + ")", true, ""}}
                              });

    connector.register_schema("market.buy", {
                                  "从商店购买",
                                  {
                                      {"shop", "商店ID(seed/grocery)", true, ""},
                                      {"item", "物品名", true, ""},
                                      {"count", "购买数量", false, "1"}
                                  }
                              });

    connector.register_schema("market.sell", {
                                  "向集市出售背包物品",
                                  {
                                      {"item", "物品名或序号", true, ""},
                                      {"count", "出售数量", false, "1"}
                                  }
                              });

    connector.register_schema("blacksmith.repair", {
                                  "在铁匠铺修复工具",
                                  {
                                      {"tool", "工具名(hoe/rod/pickaxe)", true, ""},
                                      {"method", "修复方式(ore/gold)", true, ""}
                                  }
                              });

    // ================= 自动时间推进（真实时间后台） =================
    // worldMutex 串行化「后台推进」与「FTXUI 命令执行」：命令处理与逐分钟更新
    // 共享 farm/market/weather/player/mining 等状态，必须互斥。
    std::mutex worldMutex;

    // ================= TUI 接线：参数收集 / 补全 / 命令分发 =================
    // TUI 交互模式下，参数收集由下述状态机（而非阻塞式 ParameterCollector）驱动：
    //   1) 用户输入动词（如 farm.sow）且该动词注册了 schema → 进入参数收集，
    //      tuiState.question 提示当前参数，completions 给出候选（随输入过滤）。
    //   2) 每按一次 Enter 收集一个参数；所有参数就绪后统一 dispatch。
    //   3) 未注册 schema 的动词（time.now / player.status / 移动 / ...）直接 dispatch。
    // 整个状态机仅在主线程（FTXUI 事件循环回调）运行，无需加锁。

    const std::vector<std::string> kKnownVerbs = {
        "time.now", "time.scale", "player.status",
        "move.up", "move.down", "move.left", "move.right",
        "farm.status", "farm.sow", "farm.water", "farm.fertilize", "farm.harvest",
        "fish.status", "fish.tick", "weather.now",
        "mine.status", "mine.start", "mine.stop",
        "market.status", "market.buy", "market.sell",
        "tools.status", "blacksmith.status", "blacksmith.repair",
        "save", "load", "help", "quit"
    };

    // 参数收集状态
    mud::cmd::Command pendingCmd;
    std::vector<mud::cmd::ParameterDef> pendingParams;
    std::size_t pendingIndex = 0;
    bool collecting = false;

    // 根据参数名生成候选补全列表（用户可直接输入或按 Tab 采用首个）。
    const auto param_choices =
        [&](const mud::cmd::ParameterDef& p) -> std::vector<std::string>
    {
        if (p.name == "layer")
            return {"0", "1", "2", "3", "4"};
        if (p.name == "crop")
            return {"cabbage", "carrot", "tomato", "pumpkin", "lingzhi"};
        if (p.name == "type")
            return {"normal", "advanced"};
        if (p.name == "shop")
            return {"seed", "grocery"};
        if (p.name == "tool")
            return {"hoe", "rod", "pickaxe"};
        if (p.name == "method")
            return {"ore", "gold"};
        if (p.name == "plot")
        {
            std::vector<std::string> out;
            for (std::size_t i = 0; i < farming.farmSize(); ++i)
                out.push_back(std::to_string(i));
            return out;
        }
        return {};
    };

    // 推进到下一个待填参数；全部填毕则 dispatch。（调用方需已持有 worldMutex）
    const auto ask_next_param = [&]()
    {
        if (pendingIndex >= pendingParams.size())
        {
            collecting = false;
            tuiState.question.clear();
            tuiState.completions.clear();
            const std::string verb = pendingCmd.verb;
            HandlerResult r = connector.dispatch(pendingCmd, ctx);
            pendingCmd = mud::cmd::Command{};
            pendingParams.clear();
            pendingIndex = 0;
            if (r == HandlerResult::UnknownCommand)
                msg("未知命令：" + verb + "（输入 help 查看）");
            return;
        }
        const auto& p = pendingParams[pendingIndex];
        tuiState.question = "请输入 " + p.name + "：" + p.prompt;
        tuiState.completions = param_choices(p);
    };

    // 开始为某个动词收集参数（调用方需已持有 worldMutex）。
    const auto start_collect = [&](const std::string& verb)
    {
        collecting = true;
        pendingCmd = mud::cmd::Command{};
        pendingCmd.verb = verb;
        pendingCmd.raw = verb;
        pendingParams = connector.get_schema(verb).parameters;
        pendingIndex = 0;
        ask_next_param();
    };

    // 用户停止需要 min 参数的命令时取消收集
    const auto cancel_collect = [&]()
    {
        collecting = false;
        pendingCmd = mud::cmd::Command{};
        pendingParams.clear();
        pendingIndex = 0;
        tuiState.question.clear();
        tuiState.completions.clear();
    };

    // ================= TUI 命令处理回调 =================
    // 返回 true 表示命令已处理（继续运行）；false 表示请求退出循环。
    const auto process_line = [&](const std::string& raw_line) -> bool
    {
        std::string line = raw_line;
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
            line.erase(line.begin());
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t'))
            line.pop_back();
        if (line.empty())
            return true;

        // q：参数收集中则取消输入；否则结束当前进行的动作
        if (line == "q" || line == "Q")
        {
            if (collecting)
            {
                cancel_collect();
                msg("已取消输入。");
                return true;
            }
            std::lock_guard<std::mutex> lock(worldMutex);
            if (tuiState.action_type == mud::tui::ActionType::Fish)
            {
                end_fishing(true);
                return true;
            }
            if (miningHandler.is_mining())
            {
                mud::cmd::Command cmd;
                cmd.verb = "mine.stop";
                cmd.raw = line;
                connector.dispatch(cmd, ctx);
                return true;
            }
            msg("没有正在进行的动作。");
            return true;
        }

        {
            std::lock_guard<std::mutex> lock(worldMutex);

            // 参数收集进行中：将本行作为当前参数的值。
            if (collecting)
            {
                if (line == "quit")
                {
                    cancel_collect();
                    msg("已取消输入。");
                    return true;
                }
                const auto& p = pendingParams[pendingIndex];
                if (!line.empty())
                {
                    pendingCmd.options[p.name] = line;
                    ++pendingIndex;
                }
                else if (!p.default_value.empty())
                {
                    pendingCmd.options[p.name] = p.default_value;
                    ++pendingIndex;
                }
                else if (!p.required)
                {
                    ++pendingIndex;
                }
                else
                {
                    ask_next_param(); // 必填且无默认：重新提示
                    return true;
                }
                ask_next_param();
                return true;
            }

            const std::string verb = parser.parse_verb_only(line);

            if (verb == "quit")
            {
                tuiState.push_log("再见！");
                return false; // 通知 GameTui 退出
            }
            if (verb == "help")
            {
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
                    "  fish.tick —— 开始钓鱼（q 结束）\n"
                    "  weather.now —— 查看天气\n"
                    "  mine.status —— 查看采矿状态\n"
                    "  mine.start —— 开始采矿（需层号，q 结束）\n"
                    "  mine.stop —— 停止采矿\n"
                    "  market.status —— 查看集市\n"
                    "  market.buy —— 购物（需商店/物品/数量）\n"
                    "  market.sell —— 出售（需物品/数量）\n"
                    "  tools.status —— 查看工具耐久\n"
                    "  blacksmith.status —— 查看铁匠铺\n"
                    "  blacksmith.repair —— 修复工具（需工具名/方式）\n"
                    "  save —— 存档  |  load —— 读档  |  quit —— 退出"
                });
                return true;
            }
            if (verb.empty())
            {
                msg("无法识别的输入：" + line);
                return true;
            }

            // 交互收集：注册了 schema 的动词先逐参数提示，不再直接执行。
            if (connector.has_schema(verb))
            {
                start_collect(verb);
                return true;
            }

            mud::cmd::Command cmd;
            cmd.verb = verb;
            cmd.raw = line;
            HandlerResult result = connector.dispatch(cmd, ctx);
            if (result == HandlerResult::UnknownCommand)
                msg("未知命令：" + verb + "（输入 help 查看）");
        }
        return true;
    };

    // ================= TUI 动态补全回调 =================
    // 空闲：按输入前缀过滤已知动词；参数收集中：过滤当前参数的候选。
    const auto completions_fn =
        [&](const std::string& input) -> std::vector<std::string>
    {
        if (input.empty())
            return {};
        if (collecting)
            return param_choices(pendingParams[pendingIndex]);
        std::vector<std::string> out;
        for (const auto& v : kKnownVerbs)
            if (v.rfind(input, 0) == 0)
                out.push_back(v);
        return out;
    };

    // 每秒后台节拍：推进世界 + 推进非阻塞动作 + 刷新 idle 提示。
    const auto world_step = [&]()
    {
        std::lock_guard<std::mutex> lock(worldMutex);
        tick_world();
        handle_action_tick();
        refresh_prompt();
    };

    // ================= TUI 宿主装配并进入事件循环 =================
    mud::tui::GameTui tui(tuiState);
    tui.set_process_line(process_line);
    tui.set_completions(completions_fn);
    tui.set_tick([&]() { refresh_prompt(); });

    // 后台线程每现实秒推进时间，经 post_background 在主线程执行 world_step。
    std::jthread timeThread([&](std::stop_token st)
    {
        while (!st.stop_requested())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (st.stop_requested()) break;
            tui.post_background([&]() { world_step(); });
        }
    });

    // 启动信息写入日志区。
    terminal.render_message(mud::view::MessageLine{
        "欢迎来到 胡萝卜山谷 MUD！输入行动名称即可，系统会逐个提示所需参数。\n"
        "输入 help 查看全部指令，quit 退出。"
    });
    render_now();
    terminal.render_map(player.GetPosition());
    refresh_prompt();

    // 游戏进行期间循环播放背景音乐（res/Famitracker_8bit.mp3）。
    // 路径经 MUDGAME_RES_DIR 编译期注入，避免依赖运行目录。
    const std::string kBgmFile = std::string(MUDGAME_RES_DIR) + "/Famitracker_8bit.mp3";
    mud::audio::MusicPlayer bgm;
    if (!bgm.start(kBgmFile))
    {
        terminal.render_message(mud::view::MessageLine{
            "提示：背景音乐加载失败，本次游戏静音（" + kBgmFile + "）。"
        });
    }

    // 阻塞运行 FTXUI 事件循环，直到 quit / Ctrl+C 触发退出。
    tui.run();

    // 退出游戏：停止背景音乐并清理后台时间线程。
    bgm.stop();
    timeThread.request_stop();
    game.endSession();
    return 0;
}
