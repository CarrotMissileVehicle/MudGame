/**
 * @file tool.h
 * @brief 工具模型（mud::tool）。
 *
 * 单件工具的等级 / 耐久状态与操作（使用、升级、修复）。
 */
#pragma once

#include <string>

#include "tools.h"

namespace mud::tool
{
    /** @brief 单件工具：持有等级与耐久，操作依据自身配置表。 */
    class Tool
    {
    public:
        explicit Tool(ToolId id);

        /** 使用一次工具：扣减耐久；已损坏（耐久不足一次使用）返回 false。 */
        bool use();

        /** 是否已损坏（耐久不足以完成一次使用）。 */
        bool is_broken() const;

        int level() const;
        int max_level() const;
        int durability() const;
        int max_durability() const;
        int durability_per_use() const;

        std::string name() const;

        /** 等级加成：level - 1（用作经验/效果乘数）。 */
        int level_bonus() const;

        /** 升级：未达上限则 level+1；已满级返回 false。 */
        bool upgrade();

        /** 修复满耐久。 */
        void repair_fully();

    private:
        ToolId id_;
        int level_;
        int durability_;
        const ToolConfig& cfg_;
    };
}