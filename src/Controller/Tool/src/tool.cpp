/**
 * @file tool.cpp
 * @brief 工具模型实现（mud::tool）及静态配置表定义。
 */
#include "tool.h"

#include <algorithm>

namespace mud::tool
{
    // 3 件工具的静态配置表，下标对应 ToolId 枚举序。
    // 升级/修复字段按 docs/proj.md（工具耐久度）配置：
    //   - 修复矿石：锄头/矿镐=黯铁矿，鱼竿=星纹银（各 2 个）
    //   - 修复基准金币：锄头 50 / 鱼竿 80 / 矿镐 80（按损耗比例折算）
    // 升级表步数 = kToolMaxLevel - 1 = 4：下标 0..3 对应等级 2→6 的逐级费用，
    // 材料与数量随等级提升递增（高段复用矿石材料）。
    const ToolConfig kToolConfigs[3] = {
        { ToolId::Hoe,     "锄头", 50, 1, kToolMaxLevel,
          {10, 30, 60, 100}, {"wood", "stone", "黯铁矿", "星纹银"}, {2, 3, 4, 5},
          "黯铁矿", 2, 50 },
        { ToolId::Rod,     "鱼竿", 40, 1, kToolMaxLevel,
          {10, 30, 60, 100}, {"wood", "stone", "星纹银", "黯铁矿"}, {2, 3, 4, 5},
          "星纹银", 2, 80 },
        { ToolId::Pickaxe, "矿镐", 20, 1, kToolMaxLevel,
          {10, 30, 60, 100}, {"wood", "stone", "黯铁矿", "星纹银"}, {2, 3, 4, 5},
          "黯铁矿", 2, 80 },
    };

    Tool::Tool(const ToolId id)
        : id_(id),
          level_(1),
          durability_(kToolConfigs[static_cast<int>(id)].max_durability),
          cfg_(kToolConfigs[static_cast<int>(id)])
    {
    }

    bool Tool::use()
    {
        if (is_broken())
            return false;
        durability_ -= cfg_.durability_per_use;
        return true;
    }

    bool Tool::is_broken() const
    {
        return durability_ < cfg_.durability_per_use;
    }

    int Tool::level() const               { return level_; }
    int Tool::max_level() const           { return cfg_.max_level; }
    int Tool::durability() const          { return durability_; }
    int Tool::max_durability() const      { return cfg_.max_durability; }
    int Tool::durability_per_use() const  { return cfg_.durability_per_use; }
    std::string Tool::name() const        { return cfg_.name; }
    int Tool::level_bonus() const         { return level_ - 1; }

    bool Tool::upgrade()
    {
        if (level_ >= cfg_.max_level)
            return false;
        ++level_;
        return true;
    }

    void Tool::repair_fully()
    {
        durability_ = cfg_.max_durability;
    }

    bool Tool::is_full() const
    {
        return durability_ >= cfg_.max_durability;
    }

    void Tool::set_level(int level)
    {
        level_ = std::clamp(level, 1, cfg_.max_level);
    }

    void Tool::set_durability(int durability)
    {
        durability_ = std::clamp(durability, 0, cfg_.max_durability);
    }

    std::string Tool::repair_ore() const      { return cfg_.repair_ore; }
    int Tool::repair_ore_count() const        { return cfg_.repair_ore_count; }
    int Tool::repair_base_gold() const        { return cfg_.repair_base_gold; }

    int Tool::gold_repair_cost() const
    {
        if (is_full()) return 0;
        const int loss = cfg_.max_durability - durability_;
        // base * 损耗占比，向上取整（满耐久为 0，已损至少收 1）
        const int cost = (cfg_.repair_base_gold * loss + cfg_.max_durability - 1) / cfg_.max_durability;
        return std::max(1, cost);
    }

    int Tool::ore_repair_cost() const
    {
        return (gold_repair_cost() + 1) / 2;
    }
}