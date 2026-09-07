/**
 * @file tool.cpp
 * @brief 工具模型实现（mud::tool）及静态配置表定义。
 */
#include "tool.h"

namespace mud::tool
{
    // 3 件工具的静态配置表，下标对应 ToolId 枚举序。
    // 升级/修复字段随保留供未来金币·背包系统接入，本次不使用。
    const ToolConfig kToolConfigs[3] = {
        { ToolId::Hoe,     "锄头", 50, 1, 5, {10, 30}, {"wood", "stone"}, {2, 3}, "copper", 2, 10 },
        { ToolId::Rod,     "鱼竿", 40, 1, 5, {10, 30}, {"wood", "stone"}, {2, 3}, "copper", 2, 10 },
        { ToolId::Pickaxe, "矿镐", 20, 1, 5, {10, 30}, {"wood", "stone"}, {2, 3}, "iron",   2, 10 },
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
}