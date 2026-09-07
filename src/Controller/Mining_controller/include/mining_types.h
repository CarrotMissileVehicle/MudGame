/**
 * @file mining_types.h
 * @brief 采矿域核心数据类型定义。
 *
 * 定义采矿状态、分层、速度、工具、上下文及产出结果的枚举/结构体。
 *
 * 依赖：time_service（time::gameMinutes）。
 */
#pragma once

#include "time_service.h"

#include <cstddef>
#include <string>

namespace mud::mining
{
    /** @brief 采矿会话状态。 */
    enum class MiningStatus
    {
        Idle,   // 空闲
        Mining  // 采矿中
    };

    /** @brief 矿区层级划分。 */
    enum class MiningLayer
    {
        Shallow,  // 浅层
        Middle,   // 中层
        Deep,     // 深层
        Crystal,  // 水晶层
        Core      // 核心层
    };

    /** @brief 采矿速度档位（数值越大越快）。 */
    enum class MiningSpeed
    {
        Core = 1,     // 核心镐：最慢
        Crystal = 3,  // 水晶镐
        Gold = 5,     // 金镐
        Silver = 9,   // 银镐
        Iron = 13,    // 铁镐：最快
    };

    /** @brief 采矿工具配置（待工具系统完善）。 */
    struct MiningTool // 等工具部分完善
    {
        MiningSpeed mining_speed = MiningSpeed::Core; // 工具速度档位（默认核心镐）
        time::gameMinutes interval{static_cast<std::int64_t>(mining_speed)}; // 单次采矿间隔（游戏分钟）
        double rare_bonus = 0.0;    // 稀有矿产加成
    };

    /** @brief 一次采矿会话的上下文（玩家等级、照明、工具）。 */
    struct MiningContext
    {
        std::size_t mining_level = 1; // 玩家当前采矿等级

        bool has_torch = false;   // 是否持有火把
        bool has_lantern = false; // 是否持有灯笼

        MiningTool tool{};        // 使用的采矿工具
    };

    /** @brief 单次采矿产出结果。 */
    struct MiningResult
    {
        std::string ore_id;     // 产出矿石 ID
        std::size_t experience = 0; // 获得经验
        std::size_t quantity = 1;   // 产出数量
    };
}