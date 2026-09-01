#include "tool_controller.h"

#include <cstddef>

ToolController::ToolController(mud::Player& player, mud::Inventory& inventory)
    : tools_{ std::vector<mud::tool::Tool>{ mud::tool::Tool(mud::tool::ToolId::Hoe) },
              std::vector<mud::tool::Tool>{ mud::tool::Tool(mud::tool::ToolId::Rod) },
              std::vector<mud::tool::Tool>{ mud::tool::Tool(mud::tool::ToolId::Pickaxe) } },
      player_(player), inventory_(inventory)
{
}

std::size_t ToolController::tool_count(mud::tool::ToolId id) const
{
    return tools_[static_cast<std::size_t>(id)].size();
}

bool ToolController::add_tool(mud::tool::ToolId id)
{
    tools_[static_cast<std::size_t>(id)].emplace_back(id);
    return true;
}

mud::tool::Tool& ToolController::tool(mud::tool::ToolId id, std::size_t slot)
{
    return tools_[static_cast<std::size_t>(id)].at(slot);
}

const mud::tool::ToolConfig& ToolController::config(mud::tool::ToolId id) const
{
    return mud::tool::kToolConfigs[static_cast<std::size_t>(id)];
}

// ---- 使用：自动选第一件未损坏的工具；全部损坏才失败 ----
bool ToolController::use_tool(mud::tool::ToolId id)
{
    for (auto& t : tools_[static_cast<std::size_t>(id)]) {
        if (!t.is_broken()) return t.use();
    }
    return false;
}

bool ToolController::use_tool(mud::tool::ToolId id, std::size_t slot)
{
    mud::tool::Tool& t = tool(id, slot);
    if (t.is_broken()) return false;
    return t.use();
}

bool ToolController::is_broken(mud::tool::ToolId id) const
{
    return broken_count(id) == tool_count(id);
}

std::size_t ToolController::broken_count(mud::tool::ToolId id) const
{
    std::size_t n = 0;
    for (const auto& t : tools_[static_cast<std::size_t>(id)]) {
        if (t.is_broken()) ++n;
    }
    return n;
}

int ToolController::level(mud::tool::ToolId id, std::size_t slot) const
{
    return tools_[static_cast<std::size_t>(id)].at(slot).level();
}

int ToolController::durability(mud::tool::ToolId id, std::size_t slot) const
{
    return tools_[static_cast<std::size_t>(id)].at(slot).durability();
}

int ToolController::max_durability(mud::tool::ToolId id, std::size_t slot) const
{
    return tools_[static_cast<std::size_t>(id)].at(slot).max_durability();
}

std::string ToolController::name(mud::tool::ToolId id) const
{
    const char* n = tools_[static_cast<std::size_t>(id)].at(0).name();
    return n ? n : "";
}

int ToolController::level_bonus(mud::tool::ToolId id, std::size_t slot) const
{
    return tools_[static_cast<std::size_t>(id)].at(slot).level_bonus();
}

// ---- 升级：先扣钱、再查材料，都够才真正升级 ----
bool ToolController::upgrade(mud::tool::ToolId id, std::size_t slot)
{
    mud::tool::Tool& t = tool(id, slot);
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
bool ToolController::repair(mud::tool::ToolId id, bool use_ore, std::size_t slot)
{
    mud::tool::Tool& t = tool(id, slot);
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