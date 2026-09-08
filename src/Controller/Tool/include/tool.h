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

        /** 当前耐久是否为满（无需修复）。 */
        bool is_full() const;

        /** 设置等级（读档还原，夹取到 [1, max_level]）。 */
        void set_level(int level);

        /** 设置耐久（读档还原，夹取到 [0, max_durability]）。 */
        void set_durability(int durability);

        /** 修复所需矿石名（配置表）。 */
        std::string repair_ore() const;

        /** 修复所需矿石数量（配置表）。 */
        int repair_ore_count() const;

        /** 修复基准金币（配置表，按损耗比例折算）。 */
        int repair_base_gold() const;

        /** 金币修复当前损耗折算费用（按损耗占比向上取整；满耐久为 0）。 */
        int gold_repair_cost() const;

        /** 矿石修复折算费用（金币费用向上取整减半）。 */
        int ore_repair_cost() const;

    private:
        ToolId id_;
        int level_;
        int durability_;
        const ToolConfig& cfg_;
    };
}