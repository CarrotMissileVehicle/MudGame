#pragma once                  // 防止头文件被重复包含

#include <string>             // 字符串类型
#include "tool.h"             // 工具 Model（单件工具的等级/耐久逻辑）
#include "tools.h"            // 工具配置表(枚举/配置数据)
#include "player.h"           // 依赖：玩家属性(金币)——占位
#include "inventory.h"        // 依赖：背包(材料)——占位

// ---- 类：工具控制器 ----
// 负责统一管理 3 件工具，并负责"升级/修复"时和 金币、背包 打交道。
// 依赖(玩家、背包)通过构造函数注入。
class ToolController {
public:   // 公开区：给其他人调用的接口
    // 构造函数：注入 玩家(看金币) 和 背包(看材料)
    ToolController(Player& player, Inventory& inventory);

    // ---- 使用相关 ----
    bool useTool(ToolId id);        // 用一次工具(内部会扣耐久)；坏了返回 false
    bool isBroken(ToolId id) const; // 这工具坏了吗

    // ---- 查询相关 ----
    int level(ToolId id) const;          // 当前等级
    int durability(ToolId id) const;     // 当前耐久
    std::string name(ToolId id) const;   // 工具名
    // 等级带来的加成(如锄头多开垦地块、鱼竿加稀有鱼概率等)
    int getLevelBonus(ToolId id) const;

    // ---- 升级 / 修复 ----
    bool upgrade(ToolId id);             // 升级：校验金币+材料后才升
    bool repair(ToolId id, bool useOre); // 修复：useOre=true 用矿石(费用减半)，否则用金币

private:   // 私有区
    Tool& tool(ToolId id);               // 按id取出对应那把工具(方便内部用)
    const ToolConfig& config(ToolId id) const;  // 取出该工具的配置表

    Tool tools_[kToolCount];   // 装着3件工具，下标对应 ToolId(0/1/2)
    Player& player_;           // 注入的玩家(引用)
    Inventory& inventory_;     // 注入的背包(引用)
};