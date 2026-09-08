/**
 * @file tool_controller.cpp
 * @brief 工具控制器实现（mud::tool）。
 */
#include "tool_controller.h"

namespace mud::tool
{
    // 默认构造 3 件工具（锄/竿/镐），以 ToolId 为键存入 map（DEF-006：
    // 按枚举键查找，消除「枚举序 == 数组下标」的隐式耦合）。
    ToolController::ToolController()
        : tools_{
              {ToolId::Hoe, Tool(ToolId::Hoe)},
              {ToolId::Rod, Tool(ToolId::Rod)},
              {ToolId::Pickaxe, Tool(ToolId::Pickaxe)}}
    {
    }

    bool ToolController::use_tool(const ToolId id)
    {
        return tools_.at(id).use();
    }

    bool ToolController::is_broken(const ToolId id) const
    {
        return tools_.at(id).is_broken();
    }

    int ToolController::durability(const ToolId id) const
    {
        return tools_.at(id).durability();
    }

    int ToolController::level_bonus(const ToolId id) const
    {
        return tools_.at(id).level_bonus();
    }

    int ToolController::level(const ToolId id) const
    {
        return tools_.at(id).level();
    }

    std::string ToolController::name(const ToolId id) const
    {
        return tools_.at(id).name();
    }

    int ToolController::max_durability(const ToolId id) const
    {
        return tools_.at(id).max_durability();
    }

    bool ToolController::is_full(const ToolId id) const
    {
        return tools_.at(id).is_full();
    }

    void ToolController::repair_full(const ToolId id)
    {
        tools_.at(id).repair_fully();
    }

    int ToolController::gold_repair_cost(const ToolId id) const
    {
        return tools_.at(id).gold_repair_cost();
    }

    int ToolController::ore_repair_cost(const ToolId id) const
    {
        return tools_.at(id).ore_repair_cost();
    }

    std::string ToolController::repair_ore(const ToolId id) const
    {
        return tools_.at(id).repair_ore();
    }

    int ToolController::repair_ore_count(const ToolId id) const
    {
        return tools_.at(id).repair_ore_count();
    }

    void ToolController::restore(const ToolId id, int level, int durability)
    {
        tools_.at(id).set_level(level);
        tools_.at(id).set_durability(durability);
    }
}
