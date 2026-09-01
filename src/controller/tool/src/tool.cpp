#include "tool.h"
#include "tools.h"

namespace mud::tool
{
    Tool::Tool(ToolId id)
        : id_(id), level_(1), durability_(kToolConfigs[static_cast<int>(id)].max_durability)
    {
    }

    bool Tool::use()
    {
        if (is_broken()) return false;
        durability_ -= durability_per_use();
        if (durability_ < 0) durability_ = 0;
        return true;
    }

    bool Tool::is_broken() const
    {
        return durability_ <= 1;
    }

    int Tool::level() const { return level_; }

    int Tool::max_level() const { return kToolConfigs[static_cast<int>(id_)].max_level; }

    int Tool::durability() const { return durability_; }

    int Tool::max_durability() const
    {
        return kToolConfigs[static_cast<int>(id_)].max_durability;
    }

    int Tool::durability_per_use() const
    {
        return kToolConfigs[static_cast<int>(id_)].durability_per_use;
    }

    const char* Tool::name() const { return kToolConfigs[static_cast<int>(id_)].name; }

    int Tool::level_bonus() const { return level_ - 1; }

    bool Tool::upgrade()
    {
        if (level_ >= max_level()) return false;
        ++level_;
        // 升级后耐久回满
        durability_ = max_durability();
        return true;
    }

    void Tool::repair_fully()
    {
        durability_ = max_durability();
    }
}