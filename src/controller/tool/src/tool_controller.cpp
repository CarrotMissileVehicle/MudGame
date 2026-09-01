#include "tool_controller.h"

#include <utility>

ToolController::ToolController(mud::Player& player, mud::Inventory& inventory)
    : tools_{ mud::tool::Tool(mud::tool::ToolId::Hoe),
              mud::tool::Tool(mud::tool::ToolId::Rod),
              mud::tool::Tool(mud::tool::ToolId::Pickaxe) },
      player_(player), inventory_(inventory)
{
}

mud::tool::Tool& ToolController::tool(mud::tool::ToolId id)
{
    return tools_[static_cast<std::size_t>(id)];
}

const mud::tool::ToolConfig& ToolController::config(mud::tool::ToolId id) const
{
    return mud::tool::kToolConfigs[static_cast<std::size_t>(id)];
}

bool ToolController::use_tool(mud::tool::ToolId id)
{
    return tool(id).use();
}

bool ToolController::is_broken(mud::tool::ToolId id) const
{
    return tools_[static_cast<std::size_t>(id)].is_broken();
}

int ToolController::level(mud::tool::ToolId id) const
{
    return tools_[static_cast<std::size_t>(id)].level();
}

int ToolController::durability(mud::tool::ToolId id) const
{
    return tools_[static_cast<std::size_t>(id)].durability();
}

std::string ToolController::name(mud::tool::ToolId id) const
{
    const char* n = tools_[static_cast<std::size_t>(id)].name();
    return n ? n : "";
}

int ToolController::level_bonus(mud::tool::ToolId id) const
{
    return tools_[static_cast<std::size_t>(id)].level_bonus();
}

// ---- 升级：先扣钱、再查材料，都够才真正升级 ----
bool ToolController::upgrade(mud::tool::ToolId id)
{
    mud::tool::Tool& t = tool(id);
    if (t.level() >= t.max_level()) return false;

    int step = t.level() - 1;                    // 等级1->2 对应第 0 格
    const auto& cfg = config(id);

    if (!player_.spend_gold(cfg.upgrade_cost[step])) return false;

    std::string material = cfg.upgrade_material[step];
    if (!inventory_.has_item(material, cfg.upgrade_material_count[step])) {
        // 材料不足：退还已扣的金币（避免玩家损失）
        player_.add_gold(cfg.upgrade_cost[step]);
        return false;
    }
    inventory_.remove_item(material, cfg.upgrade_material_count[step]);

    return t.upgrade();
}

// ---- 修复：可用矿石（费用减半）或金币 ----
bool ToolController::repair(mud::tool::ToolId id, bool use_ore)
{
    mud::tool::Tool& t = tool(id);
    if (t.durability() == t.max_durability()) return false;   // 满耐久无需修

    const auto& cfg = config(id);
    if (use_ore) {
        if (!inventory_.has_item(cfg.repair_ore, cfg.repair_ore_count)) return false;
        inventory_.remove_item(cfg.repair_ore, cfg.repair_ore_count);
    } else {
        // 金币修复：按损耗比例收费
        int loss = t.max_durability() - t.durability();
        int cost = cfg.repair_base_gold * loss / t.max_durability();
        if (cost < 1) cost = 1;
        if (!player_.spend_gold(cost)) return false;
    }
    t.repair_fully();
    return true;
}