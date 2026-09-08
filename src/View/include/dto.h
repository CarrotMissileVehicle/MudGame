/**
 * @file dto.h
 * @brief View 层数据传输对象（DTO）：组合根装配、面板渲染的纯数据结构。
 *
 * Controller 侧填充字段，View 侧只读渲染——View 不反查任何业务模块。
 * 纯数据 + 聚合，无行为；字段类型对齐各面板渲染需要。
 */
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "PositionCode.h"
#include "PlayerState.h"
#include "game_time.h"

namespace mud::view
{
    /** @brief 通用反馈消息：一行一条，逐行渲染。 */
    using MessageLine = std::vector<std::string>;

    // ---- 时间 ----
    struct TimeView
    {
        int year = 0;
        int month = 1;
        int day = 1;
        int hour = 0;
        int minute = 0;
        double time_scale = 1.0;
    };

    // ---- 天气 ----
    struct WeatherView
    {
        std::string weather;
        bool can_fish = true;
        bool can_go_outside = true;
        bool auto_water = false;
        double crop_loss_rate = 0.0;
        double mining_exp_bonus = 0.0;
        double fishing_penalty = 0.0;
        std::vector<std::string> events;
    };

    // ---- 玩家状态 ----
    struct PlayerStatus
    {
        PositionCode position = AtHome;
        StateCode state = Waiting;
        int satiety = 0;
        int maxSatiety = 0;
        int farmingExp = 0;
        int fishExp = 0;
        int mineExp = 0;
        std::vector<std::string> bagItems;
    };

    // ---- 农田 ----
    struct PlotView
    {
        std::size_t index = 0;
        bool occupied = false;
        std::string crop_name;
        int growth_stage = 0;
        int growth_max = 0;
        bool watered = false;
    };

    struct FarmView
    {
        std::vector<PlotView> plots;
    };

    // ---- 钓鱼 ----
    struct FishPoolView
    {
        std::string name;
        float probability = 0.0f;
    };

    struct FishingView
    {
        std::vector<FishPoolView> pool;
        bool can_fish = true;
    };

    // ---- 集市 ----
    struct MarketItemView
    {
        std::string name;
        int buy = 0;
        int sell = 0;
    };

    struct ShopView
    {
        std::string id;
        std::string name;
        std::vector<MarketItemView> items;
    };

    struct MarketView
    {
        long long gold = 0;
        int day_of_week = 1;
        bool prosperous = false;
        bool festival = false;
        std::vector<ShopView> shops;
    };

    // ---- 工具 ----
    struct ToolView
    {
        std::string name;
        int durability = 0;
        int level = 1;
        bool broken = false;
    };

    struct ToolsView
    {
        std::vector<ToolView> tools;
    };

    struct ToolRepairView
    {
        std::string name;
        int durability = 0;
        int max_durability = 0;
        bool broken = false;
        int repair_gold = 0;
        std::string repair_ore;
        int repair_ore_needed = 0;
        int repair_ore_held = 0;
    };

    struct BlacksmithView
    {
        long long gold = 0;
        std::vector<ToolRepairView> tools;
    };

    // ---- 采矿 ----
    struct MiningView
    {
        bool is_mining = false;
        std::size_t layer = 0;
        mud::time::GameDateTime start_time{};
        int mining_level = 1;
    };

    struct MiningEventView
    {
        std::string ore_name;
        std::size_t quantity = 0;
        std::size_t experience = 0;
    };

    // ---- 整帧快照（一次性全量渲染）----
    struct GameSnapshot
    {
        MessageLine messages;
        TimeView time;
        WeatherView weather;
        PlayerStatus player;
        FarmView farm;
        MarketView market;
        ToolsView tools;
        MiningView mining;
        FishingView fishing;
    };
} // namespace mud::view
