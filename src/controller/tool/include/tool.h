#pragma once

#include <string>

namespace mud::tool
{
    enum class ToolId
    {
        Hoe = 0,      // 锄头
        Rod = 1,      // 鱼竿
        Pickaxe = 2   // 矿镐
    };

    // 工具静态配置表
    struct ToolConfig
    {
        ToolId id;
        const char* name;
        int max_durability;        // 初始/最大耐久
        int durability_per_use;    // 每次使用消耗
        int max_level;             // 最高等级
        // 升级费用（下标 = 等级-1，Lv1->2 在第 0 格）
        int upgrade_cost[2];
        // 升级材料 id 及数量
        const char* upgrade_material[2];
        int upgrade_material_count[2];
        // 矿修材料及数量
        const char* repair_ore;
        int repair_ore_count;
        // 金币修复基准费用（按损耗比例计）
        int repair_base_gold;
    };

    // 单件工具 Model：等级、耐久的纯逻辑
    class Tool
    {
    public:
        explicit Tool(ToolId id);

        bool use();                 // 使用一次：扣耐久；损坏时返回 false
        bool is_broken() const;     // 耐久 <= 1 时视为损坏
        int level() const;
        int max_level() const;
        int durability() const;
        int max_durability() const;
        int durability_per_use() const;
        const char* name() const;
        int level_bonus() const;    // 额外加成 = 等级 - 1

        bool upgrade();             // 提升一级
        void repair_fully();        // 耐久回满

    private:
        ToolId id_;
        int level_;
        int durability_;
    };
}