/**
 * @file tool_controller.h
 * @brief 工具控制器（mud::tool）。
 *
 * 统一管理 3 件工具（锄/竿/镐）：使用、耐久查询、等级加成、
 * 铁匠铺修复成本计算与读档还原。
 * 修复/还原不涉及金币与背包校验——由调用方（组合根）完成
 * 金钱与矿石的扣减后再调用，保持控制器自包含。
 */
#pragma once

#include <string>

#include "tool.h"
#include "tools.h"

namespace mud::tool
{
    /** @brief 工具控制器：管理 3 件工具，聚焦采矿所需能力与修复。 */
    class ToolController
    {
    public:
        ToolController();

        /** 使用指定工具一次（扣耐久）；该工具损坏返回 false。 */
        bool use_tool(ToolId id);

        /** 指定工具是否已损坏。 */
        bool is_broken(ToolId id) const;

        /** 指定工具当前耐久。 */
        int durability(ToolId id) const;

        /** 指定工具满耐久。 */
        int max_durability(ToolId id) const;

        /** 指定工具是否满耐久。 */
        bool is_full(ToolId id) const;

        /** 等级加成（level - 1）。 */
        int level_bonus(ToolId id) const;

        /** 当前等级。 */
        int level(ToolId id) const;

        /** 工具名。 */
        std::string name(ToolId id) const;

        /** 全额修复指定工具（不扣金币/材料，由调用方完成校验与扣减）。 */
        void repair_full(ToolId id);

        /** 金币修复该工具当前损耗折算费用（满耐久为 0）。 */
        int gold_repair_cost(ToolId id) const;

        /** 矿石修复折算费用（金币费用向上取整减半）。 */
        int ore_repair_cost(ToolId id) const;

        /** 修复所需矿石名。 */
        std::string repair_ore(ToolId id) const;

        /** 修复所需矿石数量。 */
        int repair_ore_count(ToolId id) const;

        /** 读档还原工具等级与耐久（夹取到合法范围）。 */
        void restore(ToolId id, int level, int durability);

    private:
        Tool tools_[3]; // 下标对应 ToolId（Hoe=0/Rod=1/Pickaxe=2）
    };
}