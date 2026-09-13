/**
 * @file GameContext.h
 * @brief 组合根的游戏上下文：聚合各子系统引用，统一装配 DTO 与反馈文本。
 *
 * 本结构体承担原 main.cpp 中 make_*_view / input_hint / refresh_prompt / msg /
 * render_now / opt_int 等职责，是命令处理器（GameCommands）与世界推进
 * （WorldEngine）共享的上下文。引用聚合、不拥有对象，生命周期与 main 一致。
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
#include "Fertilizer.h"
#include "Fish.h"
#include "FishingController.h"
#include "Market.h"
#include "weather_controller.h"
#include "tool_controller.h"
#include "ore_data.h"
#include "mining_handler.h"
#include "TuiState.h"
#include "dto.h"
#include "connector.h"
#include "TerminalView.h"

/**
 * @brief 游戏上下文：持有全部子系统引用，对外提供 DTO 装配与反馈输出。
 *
 * 引用成员按声明顺序由 GameSession 以聚合初始化装配；
 * View 层只读 DTO，Controller 逻辑经本类访问共享世界状态。
 */
struct GameContext
{
    // ---- 子系统引用（引用聚合）----
    Game& game;
    mud::TimeService& time;
    Player& player;
    Move& move;
    Farm& farm;
    FarmingController& farming;
    const std::map<std::string, Crop*>& seeds;
    const std::map<const Crop*, std::string>& crop_names;
    Fertilizer& normal_fert;   // 普通肥料原型（堆栈持有，地块仅保存非拥有指针）
    Fertilizer& advanced_fert; // 高级肥料原型
    const std::vector<Fish*>& fish_pool;
    const std::map<const Fish*, std::string>& fish_names;
    FishingController& fishing;
    Market& market;
    int& gold;
    WeatherController& weather;
    mud::tool::ToolController& tools;
    Ore& ore_table;
    MiningController& mining_controller;
    MiningHandler& mining;
    mud::tui::TuiState& tui;
    mud::view::TerminalView& view;

    // ---- DTO 装配（组合根 → View 的唯一数据通道）----
    mud::view::TimeView make_time_view() const;
    mud::view::WeatherView make_weather_view() const;
    PlayerStatus make_player_status() const;
    mud::view::FarmView make_farm_view() const;
    mud::view::FishingView make_fishing_view() const;
    mud::view::MarketView make_market_view() const;
    mud::view::ToolsView make_tools_view() const;
    mud::view::MiningView make_mining_view() const;
    mud::view::BlacksmithView make_blacksmith_view() const;

    // ---- 交互辅助 ----
    std::string input_hint() const;      // 按所在位置给出可用指令提示
    void refresh_prompt() const;         // 刷新 TUI 输入法提示
    void msg(const std::string& text) const;  // 单行反馈 → MessagePanel
    void render_now() const;             // 通用输出：当前时刻 + 天气

    /// 从命令 options 读取整数型命名参数，缺省返回 fallback。
    long long opt_int(const mud::cmd::Command& cmd,
                      const std::string& key, long long fallback) const;
};