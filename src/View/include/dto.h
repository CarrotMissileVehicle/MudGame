/**
 * @file dto.h
 * @brief View 层只读数据传输对象（DTO）集合。
 *
 * 铁律：DTO 是 View 的唯一输入。Controller/组合根装配 DTO，View 只读。
 * 本头只依赖纯类型（枚举 / 时间结构），严禁 include 任何业务类
 * （Player/Farm/Market/TimeService 等），否则破坏 MVC 单向依赖。
 */
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "PositionCode.h"
#include "PlayerStateCode.h"
#include "game_time.h"

/**
 * @brief 玩家状态快照（全局命名空间，沿用既有类型）。
 * 由组合根从 Player + Bag 装配，StatusPanel 只读渲染。
 */
struct PlayerStatus
{
    // DEF-410：全部字段显式默认初始化，装配方漏填任一字段也不会渲染随机脏数据
    PositionCode position = AtHome;
    StateCode state = StateCode::Waiting;
    int satiety = 0;
    int maxSatiety = 0;
    int farmingExp = 0;
    int fishExp = 0;
    int mineExp = 0;
    std::vector<std::string> bagItems;
};

namespace mud::view
{
    /** @brief 时间面板数据（来自 TimeService::now / time_scale）。 */
    struct TimeView
    {
        int year = 0, month = 1, day = 1, hour = 0, minute = 0;
        double time_scale = 60.0;
    };

    /** @brief 天气面板数据（来自 WeatherController 各查询方法）。 */
    struct WeatherView
    {
        std::string weather;
        bool can_fish = false;
        bool can_go_outside = true;
        bool auto_water = false;
        double crop_loss_rate = 0.0;
        double mining_exp_bonus = 0.0;
        double fishing_penalty = 0.0;
        std::vector<std::string> events;
    };

    /** @brief 单个地块状态摘要（来自 Farm::getFarmland）。 */
    struct PlotView
    {
        std::size_t index = 0;
        bool occupied = false;
        std::string crop_name;
        int growth_stage = 0;
        int growth_max = 0;
        bool watered = false;
    };

    /** @brief 农田面板数据。 */
    struct FarmView
    {
        std::vector<PlotView> plots;
    };

    /** @brief 鱼池条目（来自 Fish::getProbability）。 */
    struct FishEntry
    {
        std::string name;
        float probability = 0.0f;
    };

    /** @brief 钓鱼面板数据。 */
    struct FishingView
    {
        std::vector<FishEntry> pool;
        bool can_fish = true;
    };

    /** @brief 商店货架条目（来自 Market 报价查询）。 */
    struct MarketItemView
    {
        std::string name;
        int buy = 0;
        int sell = 0;
    };

    /** @brief 单个商店视图。 */
    struct ShopView
    {
        std::string id;
        std::string name;
        std::vector<MarketItemView> items;
    };

    /** @brief 集市面板数据。 */
    struct MarketView
    {
        int gold = 0;
        int day_of_week = 1;
        bool prosperous = false;
        bool festival = false;
        std::vector<ShopView> shops;
    };

    /** @brief 单把工具视图。 */
    struct ToolView
    {
        std::string name;
        int durability = 0;
        int level = 1;
        bool broken = false;
    };

    /** @brief 工具面板数据。 */
    struct ToolsView
    {
        std::vector<ToolView> tools;
    };

    /** @brief 铁匠铺单件工具的修复条目。 */
    struct ToolRepairView
    {
        std::string name;
        int durability = 0;
        int max_durability = 0;
        bool broken = false;
        int repair_gold = 0;        // 当前损耗折算金币修复费用
        std::string repair_ore;     // 修复所需矿石名
        int repair_ore_needed = 0;  // 所需矿石数量
        int repair_ore_held = 0;    // 背包持有矿石数量
    };

    /** @brief 铁匠铺面板数据。 */
    struct BlacksmithView
    {
        int gold = 0;
        std::vector<ToolRepairView> tools;
    };

    /** @brief 采矿会话/状态视图。 */
    struct MiningView
    {
        bool is_mining = false;
        std::size_t layer = 0;
        mud::time::GameDateTime start_time;
        std::size_t mining_level = 1;
    };

    /** @brief 采矿产出入库反馈（来自 grant_mining）。 */
    struct MiningEventView
    {
        std::string ore_name;
        std::size_t quantity = 0;
        std::size_t experience = 0;
    };

    /** @brief 操作反馈集合：Controller 把结果文本交给 View，View 只负责展示。 */
    using MessageLine = std::vector<std::string>;

    /** @brief 全量游戏快照：组合根装配一次，TerminalView::render_all 一次性渲染。 */
    struct GameSnapshot
    {
        TimeView time;
        PlayerStatus player;
        WeatherView weather;
        FarmView farm;
        FishingView fishing;
        MarketView market;
        ToolsView tools;
        MiningView mining;
        MessageLine messages;
    };
} // namespace mud::view