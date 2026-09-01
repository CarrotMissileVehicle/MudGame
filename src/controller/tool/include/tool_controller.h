#pragma once

// ==========================================================================
// 工具 Controller（mud 风格）
// 职责：持有并管理各类型工具，每类可"同时持有多件"（工具是一种物品，继承自
//       Object）。使用时自动选一件未损坏的，故一件耐久用尽时可换另一件继续
//       使用，不必急着修复。
// 依赖：Player / Inventory 通过构造函数注入（目前是占位桩）。
// ==========================================================================

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "tool.h"      // mud::tool
#include "tools.h"     // mud::tool::kToolConfigs / kToolCount
#include "player.h"    // mud::Player（留桩）
#include "inventory.h" // mud::Inventory（留桩）

class ToolController
{
public:
    ToolController(mud::Player& player, mud::Inventory& inventory);

    // ---- 持有 ----
    std::size_t tool_count(mud::tool::ToolId id) const; // 该类型当前持有件数
    bool add_tool(mud::tool::ToolId id);                // 新增一件同类型工具

    // ---- 使用（自动选一件未损坏的）----
    bool use_tool(mud::tool::ToolId id);                   // 自动选第一件未损坏的；全部损坏则 false
    bool use_tool(mud::tool::ToolId id, std::size_t slot); // 使用指定件

    bool is_broken(mud::tool::ToolId id) const;            // 该类型是否已无可用工具（全部损坏）
    std::size_t broken_count(mud::tool::ToolId id) const;  // 该类型已损坏件数

    // ---- 查询（slot 默认主件 = 0）----
    int  level(mud::tool::ToolId id, std::size_t slot = 0) const;
    int  durability(mud::tool::ToolId id, std::size_t slot = 0) const;
    int  max_durability(mud::tool::ToolId id, std::size_t slot = 0) const;
    int  level_bonus(mud::tool::ToolId id, std::size_t slot = 0) const;
    std::string name(mud::tool::ToolId id) const;

    // ---- 升级 / 修复（slot 默认主件 = 0）----
    bool upgrade(mud::tool::ToolId id, std::size_t slot = 0);
    bool repair(mud::tool::ToolId id, bool use_ore, std::size_t slot = 0);

private:
    mud::tool::Tool& tool(mud::tool::ToolId id, std::size_t slot);  // 按 id+slot 取工具
    const mud::tool::ToolConfig& config(mud::tool::ToolId id) const;

    std::array<std::vector<mud::tool::Tool>, mud::tool::kToolCount> tools_;
    mud::Player&     player_;
    mud::Inventory&  inventory_;
};