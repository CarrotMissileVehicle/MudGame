#pragma once

// ==========================================================================
// 工具 Controller（mud 风格）
// 职责：管理 3 件工具，并在"升级/修复"时和 金币、背包 打交道。
// 依赖：Player / Inventory 通过构造函数注入（目前是占位桩）。
// ==========================================================================

#include <string>

#include "tool.h"      // mud::tool
#include "tools.h"     // mud::tool::kToolConfigs / kToolCount
#include "player.h"    // mud::Player（留桩）
#include "inventory.h" // mud::Inventory（留桩）

class ToolController
{
public:
    ToolController(mud::Player& player, mud::Inventory& inventory);

    // ---- 使用相关 ----
    bool use_tool(mud::tool::ToolId id);        // 用一次工具；坏了返回 false
    bool is_broken(mud::tool::ToolId id) const;

    // ---- 查询相关 ----
    int  level(mud::tool::ToolId id) const;
    int  durability(mud::tool::ToolId id) const;
    std::string name(mud::tool::ToolId id) const;
    int  level_bonus(mud::tool::ToolId id) const;

    // ---- 升级 / 修复 ----
    bool upgrade(mud::tool::ToolId id);              // 校验金币+材料后升一级
    bool repair(mud::tool::ToolId id, bool use_ore); // use_ore=true 用矿石(费用减半)，否则用金币

private:
    mud::tool::Tool& tool(mud::tool::ToolId id);   // 按 id 取出对应工具
    const mud::tool::ToolConfig& config(mud::tool::ToolId id) const;

    mud::tool::Tool tools_[mud::tool::kToolCount];
    mud::Player&     player_;
    mud::Inventory&  inventory_;
};