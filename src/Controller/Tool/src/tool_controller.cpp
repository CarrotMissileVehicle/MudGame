/**
 * @file tool_controller.cpp
 * @brief 工具控制器实现（mud::tool）。
 */
#include "tool_controller.h"

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

    int ToolController::max_durability(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].max_durability();
    }

    bool ToolController::is_full(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].is_full();
    }

    void ToolController::repair_full(const ToolId id)
    {
        tools_[static_cast<int>(id)].repair_fully();
    }

    int ToolController::gold_repair_cost(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].gold_repair_cost();
    }

    int ToolController::ore_repair_cost(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].ore_repair_cost();
    }

    std::string ToolController::repair_ore(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].repair_ore();
    }

    int ToolController::repair_ore_count(const ToolId id) const
    {
        return tools_[static_cast<int>(id)].repair_ore_count();
    }

    void ToolController::restore(const ToolId id, int level, int durability)
    {
        tools_[static_cast<int>(id)].set_level(level);
        tools_[static_cast<int>(id)].set_durability(durability);
    }
}