/**
 * @file tool_Controller.h
 * @brief 工具控制器（mud::tool）。
 *
 * 统一管理 3 件工具（锄/竿/镐）：使用、耐久查询、等级加成。
 * 升级/修复接口暂不提供（依赖金币/背包桩未就绪，属留白）。
 */
#pragma once

#include <string>

#include "tool.h"
#include "tools.h"

namespace mud::tool
{
    /** @brief 工具控制器：管理 3 件工具，聚焦采矿所需能力。 */
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

        /** 指定工具等级加成（level - 1）。 */
        int level_bonus(ToolId id) const;

        /** 指定工具当前等级。 */
        int level(ToolId id) const;

        /** 指定工具名。 */
        std::string name(ToolId id) const;

    private:
        Tool tools_[3]; // 下标对应 ToolId（Hoe=0/Rod=1/Pickaxe=2）
    };
}