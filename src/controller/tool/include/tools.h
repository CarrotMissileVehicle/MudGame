#pragma once

#include <cstddef>
#include "tool.h"

namespace mud::tool
{
    // 工具配置表，下标与 ToolId 对应（0/1/2）
    // 升级费用/材料参照 subsys.md
    inline constexpr ToolConfig kToolConfigs[] = {
        // Hoe 锄头
        {
            ToolId::Hoe, "锄头",
            50, 1, 3,                          // 耐久50 每次-1 最高3级
            { 50, 100 },                       // Lv1->2:50金, Lv2->3:100金
            { "ore_iron", "ore_silver" },      // 材料
            { 5, 3 },
            "ore_iron", 2,                     // 矿修：黯铁矿×2
            50,                                // 修复基准 50
        },
        // Rod 鱼竿
        {
            ToolId::Rod, "鱼竿",
            30, 1, 3,
            { 80, 200 },
            { "ore_iron", "ore_silver" },
            { 5, 3 },
            "ore_silver", 2,                   // 矿修：星纹银×2
            80,
        },
        // Pickaxe 矿镐
        {
            ToolId::Pickaxe, "矿镐",
            25, 2, 3,
            { 80, 200 },
            { "ore_iron", "ore_silver" },
            { 5, 3 },
            "ore_iron", 2,                     // 矿修：黯铁矿×2
            80,
        },
    };
    inline constexpr std::size_t kToolCount = sizeof(kToolConfigs) / sizeof(kToolConfigs[0]);
}