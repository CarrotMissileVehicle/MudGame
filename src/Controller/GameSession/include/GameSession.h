/**
 * @file GameSession.h
 * @brief 游戏会话装配（组合根核心）：持有全部子系统并构建 GameContext。
 *
 * 对应原 main.cpp 中从 Game 到 Connector 的全部对象装配。构造完成后 main
 * 仅需取用 context() / connector() / tui_state() 并接线 TUI 与后台线程。
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "Game.h"
#include "time_service.h"
#include "Player.h"
#include "Move.h"
#include "farm.h"
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
#include "weather_controller.h"
#include "tool_controller.h"
#include "ore_data.h"
#include "mining_controller.h"
#include "mining_handler.h"
#include "event.h"
#include "TuiState.h"
#include "TuiRenderer.h"
#include "TerminalView.h"
#include "connector.h"
#include "GameContext.h"

/**
 * @brief 游戏会话：按构造函数注入装配全部子系统（生命周期与 main 一致）。
 *
 * 成员声明顺序即构造顺序；GameContext 为引用聚合，聚合初始化引用各成员。
 */
class GameSession
{
public:
    GameSession();
    ~GameSession();

    // 组合根持有相互引用的子系统成员（seeds_→cabbage_…、move_{&player_} 等），
    // 拷贝/移动会使成员引用别名悬垂，禁止一切复制与搬移。
    GameSession(const GameSession&) = delete;
    GameSession& operator=(const GameSession&) = delete;
    GameSession(GameSession&&) = delete;
    GameSession& operator=(GameSession&&) = delete;

    GameContext& context() { return context_; }
    mud::tui::TuiState& tui_state() { return tuiState_; }
    mud::view::TerminalView& terminal() { return terminal_; }
    Connector& connector() { return connector_; }

private:
    // ---- 时间（唯一时间真相）----
    Game game_;
    mud::TimeService timeService_;

    // ---- 玩家 ----
    Player player_;
    Move move_;

    // ---- 农田 / 作物 / 肥料原型（堆栈持有，地块仅保存非拥有指针）----
    Farm farm_;
    FarmingController farming_;
    Cabbage cabbage_;
    Carrot carrot_;
    Tomato tomato_;
    Pumpkin pumpkin_;
    Lingzhi lingzhi_;
    NormalFertilizer normalFert_;
    AdvancedFertilizer advancedFert_;
    const std::map<std::string, Crop*> seeds_;
    const std::map<const Crop*, std::string> cropNames_;

    // ---- 鱼池（堆栈持有，钓到即复制入背包，绝不共享指针）----
    Crucian crucian_;
    GrassCarp grassCarp_;
    Perch perch_;
    RainbowTrout rainbowTrout_;
    KingCrab kingCrab_;
    const std::vector<Fish*> fishPool_;
    const std::map<const Fish*, std::string> fishNames_;
    FishingController fishing_;

    // ---- 集市（商店货架为独立具名 Object 实例，购买会向背包复制新对象）----
    Object seedCabbage_;
    Object seedCarrot_;
    Object seedTomato_;
    Object seedPumpkin_;
    Object seedLingzhi_;
    Object fertNormal_;
    Object fertAdvanced_;
    Market market_;
    Shop seedShop_;
    Shop groceryShop_;
    Shop blacksmithShop_;
    int gold_ = 200;

    // ---- 天气 ----
    WeatherController weather_;

    // ---- 工具 / 采矿 ----
    mud::tool::ToolController tools_;
    Ore::OreData oreData_;       // 控制器仅持引用、无需数据文件内容
    Ore oreTable_;               // 供玩家反馈名称/价格/用途
    mud::event::EventSystem miningEvents_;
    MiningController miningController_;
    MiningHandler miningHandler_;

    // ---- 视图（View 层唯一输出出口）----
    mud::tui::TuiState tuiState_;
    mud::tui::TuiRenderer outRenderer_;
    mud::view::TerminalView terminal_;

    // ---- 解析 / 派发 ----
    Connector connector_;

    // ---- 组合根上下文（引用聚合，最后初始化）----
    GameContext context_;
};