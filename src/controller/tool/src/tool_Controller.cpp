#include "tool_Controller.h"   // 实现这个头文件声明的方法

// ---- 构造函数实现 ----
// 用初始化列表：
//   tools_ 数组里依次放锄头/鱼竿/矿镐三件(每件都走 Tool 构造函数，从1级满耐久开始)
//   player_ = 传入的玩家引用；inventory_ = 传入的背包引用
ToolController::ToolController(Player& player, Inventory& inventory)
    : tools_{ Tool(ToolId::HOE), Tool(ToolId::ROD), Tool(ToolId::PICKAXE) },
      player_(player), inventory_(inventory) {}

// ---- 私有辅助：按 id 取出对应那件工具 ----
// static_cast<int>(id) 把枚举转成整数当数组下标。
// 返回引用(&) → 外部能直接修改那件工具本体。
Tool& ToolController::tool(ToolId id) {
    return tools_[static_cast<int>(id)];
}

// ---- 私有辅助：取出该工具的静态配置表 ----
const ToolConfig& ToolController::config(ToolId id) const {
    return kToolConfigs(static_cast<int>(id));
}

// 使用工具：直接转发给对应工具自己的 use() 方法
bool ToolController::useTool(ToolId id) {
    return tool(id).use();
}

// 查询是否损坏
bool ToolController::isBroken(ToolId id) const {
    return tools_[static_cast<int>(id)].isBroken();
}

int ToolController::level(ToolId id) const {
    return tools_[static_cast<int>(id)].level();
}

int ToolController::durability(ToolId id) const {
    return tools_[static_cast<int>(id)].durability();
}

std::string ToolController::name(ToolId id) const {
    return tools_[static_cast<int>(id)].getName();
}

// 等级加成：等级从1开始，额外加成 = 超出的等级数(等级2→加成1，等级3→加成2)
int ToolController::getLevelBonus(ToolId id) const {
    return level(id) - 1;
}

// ---- 升级：先检查钱和材料，够了才真的升级 ----
bool ToolController::upgrade(ToolId id) {
    Tool& t = tool(id);                       // 拿到要升级的工具
    if (t.level() >= t.maxLevel()) return false;  // 已满级 → 失败

    // step 是"第几步升级"：等级1→2是第0步，对应配置数组第0格
    int step = t.level() - 1;
    const ToolConfig& cfg = config(id);       // 该工具的配置

    // 1) 先扣钱：spendGold 会自己判断钱够不够，不够返回 false
    //    "!" 是取反，所以 !false = true 才会进入 return false(失败)
    if (!player_.spendGold(cfg.upgrade_cost[step])) return false;

    // 2) 再检查材料够不够
    if (!inventory_.hasItem(cfg.upgrade_material[step], cfg.upgrade_material_count[step])) {
        return false;   // 材料不够 → 失败（钱已经扣了，此处是简化处理）
    }
    // 3) 材料够，就从背包里扣掉
    inventory_.removeItem(cfg.upgrade_material[step], cfg.upgrade_material_count[step]);

    return t.upgrade();   // 4) 真正执行升级
}

// ---- 修复 ----
bool ToolController::repair(ToolId id, bool useOre) {
    Tool& t = tool(id);                              // 拿到要修的工具
    // 如果既没坏 又已是满耐久 → 无需修，直接返回 false
    if (!t.isBroken() && t.durability() == t.maxDurability()) return false;

    const ToolConfig& cfg = config(id);
    if (useOre) {
        // 方式1：用矿石修(费用减半的原理就是只花材料不花金币)
        if (!inventory_.hasItem(cfg.repair_ore, cfg.repair_ore_count)) return false;
        inventory_.removeItem(cfg.repair_ore, cfg.repair_ore_count);
    } else {
        // 方式2：用金币修，钱按"损耗了多少"来算
        int loss = t.maxDurability() - t.durability();        // 损耗量
        int cost = cfg.repair_base_gold * loss / t.maxDurability();  // 按比例算钱
        if (cost < 1) cost = 1;   // 至少收1金币，避免出现0元
        if (!player_.spendGold(cost)) return false;   // 钱不够 → 失败
    }
    t.repairFully();   // 真正把耐久修满
    return true;
}