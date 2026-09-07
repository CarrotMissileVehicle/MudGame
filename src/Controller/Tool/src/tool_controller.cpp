/**
 * @file tool_Controller.cpp
 * @brief 工具控制器实现（mud::tool）。
 */
#include "tool_Controller.h"

namespace mud::tool
{
    // 默认构造 3 件工具（锄/竿/镐），下标对应 ToolId。
    ToolController::ToolController()
        : tools_{ Tool(ToolId::Hoe), Tool(ToolId::Rod), Tool(ToolId::Pickaxe) }
    {
    }

    bool ToolController::use_tool(const ToolId id)
    {
        return tools_[static_cast<int>(id)].use();
    }

    bool ToolController::is_broken(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].is_broken();
    }

    int ToolController::durability(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].durability();
    }

    int ToolController::level_bonus(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].level_bonus();
    }

    int ToolController::level(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].level();
    }

    std::string ToolController::name(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].name();
    }
}