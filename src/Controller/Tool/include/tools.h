/**
 * @file tools.h
 * @brief 工具配置表（mud::tool）。
 *
 * 定义工具标识枚举与静态配置表；配置表下标即 ToolId 枚举序（Hoe=0/Rod=1/Pickaxe=2）。
 */
#pragma once

namespace mud::tool
{
    /** @brief 工具标识。作为整数即 kToolConfigs 数组下标。 */
    enum class ToolId : int { Hoe = 0, Rod = 1, Pickaxe = 2 };

    /** @brief 单件工具的静态配置。 */
    struct ToolConfig
    {
        ToolId id;
        const char* name;          // 工具名
        int max_durability;        // 满耐久
        int durability_per_use;    // 每次使用消耗耐久
        int max_level;             // 等级上限
        int upgrade_cost[2];              // 两级升级各自金币消耗
        const char* upgrade_material[2]; // 两级升级各自材料名
        int upgrade_material_count[2];    // 两级升级各自材料数量
        const char* repair_ore;    // 修复所需矿石名
        int repair_ore_count;      // 修复所需矿石数量
        int repair_base_gold;      // 修复基准金币（按损耗比例折算）
    };

    /** @brief 3 件工具的静态配置表，下标对应 ToolId 枚举序。 */
    extern const ToolConfig kToolConfigs[3];
}