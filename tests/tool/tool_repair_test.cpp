/**
 * @file tool_repair_test.cpp
 * @brief 工具修复逻辑测试。
 *
 * 覆盖：损坏判定、金币/矿石修复费用折算（满耐久为 0、按损耗比例向上取整）、
 * 修复满耐久、矿石名/数量配置、读档还原夹取（set_level / set_durability）、
 * ToolController 代理方法。
 */
#include <gtest/gtest.h>

#include "tool.h"
#include "tool_controller.h"
#include "tools.h"

using mud::tool::Tool;
using mud::tool::ToolId;
using mud::tool::ToolController;

TEST(ToolRepairTest, FullDurabilityCostsZero)
{
    Tool hoe(ToolId::Hoe);
    EXPECT_EQ(hoe.gold_repair_cost(), 0);
    EXPECT_EQ(hoe.ore_repair_cost(), 0);
    EXPECT_TRUE(hoe.is_full());
}

TEST(ToolRepairTest, PartialDurabilityCostsScaleByLoss)
{
    Tool pick(ToolId::Pickaxe);
    const int max = pick.max_durability();
    const int base = pick.repair_base_gold();
    pick.set_durability(max / 2);
    EXPECT_FALSE(pick.is_full());
    // 损耗 50%：费用按损耗占比向上取整
    const int expected = static_cast<int>((base * (max - max / 2) + max - 1) / max);
    EXPECT_EQ(pick.gold_repair_cost(), std::max(1, expected));
    // 矿石费用 = 金币费用向上取整减半
    EXPECT_EQ(pick.ore_repair_cost(), (pick.gold_repair_cost() + 1) / 2);
}

TEST(ToolRepairTest, RepairFullyRestoresDurability)
{
    Tool rod(ToolId::Rod);
    for (int i = 0; i < 5; ++i) (void)rod.use();
    EXPECT_TRUE(rod.is_broken() || rod.durability() < rod.max_durability());
    rod.repair_fully();
    EXPECT_TRUE(rod.is_full());
    EXPECT_EQ(rod.durability(), rod.max_durability());
}

TEST(ToolRepairTest, RepairOreConfigMatchesDesign)
{
    // 设计：锄头/矿镐用黯铁矿，鱼竿用星纹银，均需 2 个
    EXPECT_EQ(Tool(ToolId::Hoe).repair_ore(), "黯铁矿");
    EXPECT_EQ(Tool(ToolId::Hoe).repair_ore_count(), 2);
    EXPECT_EQ(Tool(ToolId::Rod).repair_ore(), "星纹银");
    EXPECT_EQ(Tool(ToolId::Rod).repair_ore_count(), 2);
    EXPECT_EQ(Tool(ToolId::Pickaxe).repair_ore(), "黯铁矿");
    EXPECT_EQ(Tool(ToolId::Pickaxe).repair_ore_count(), 2);
}

TEST(ToolRepairTest, RestoreClampsLevelAndDurability)
{
    Tool hoe(ToolId::Hoe);
    hoe.set_level(99);            // 越上限 → 夹取到 1（Hoe 配置内按 1 处理上限由配置决定）
    EXPECT_GE(hoe.level(), 1);
    hoe.set_durability(-5);       // 越下限 → 夹取到 0
    EXPECT_EQ(hoe.durability(), 0);
    EXPECT_TRUE(hoe.is_broken());
    hoe.set_durability(100000);   // 越上限 → 夹取到满耐久
    EXPECT_EQ(hoe.durability(), hoe.max_durability());
}

TEST(ToolControllerTest, ProxyCostAndRepair)
{
    ToolController tools;
    tools.use_tool(ToolId::Pickaxe);              // 扣一次耐久
    EXPECT_FALSE(tools.is_full(ToolId::Pickaxe));
    EXPECT_GT(tools.gold_repair_cost(ToolId::Pickaxe), 0);
    EXPECT_EQ(tools.ore_repair_cost(ToolId::Pickaxe),
              (tools.gold_repair_cost(ToolId::Pickaxe) + 1) / 2);
    tools.repair_full(ToolId::Pickaxe);
    EXPECT_TRUE(tools.is_full(ToolId::Pickaxe));
    EXPECT_EQ(tools.gold_repair_cost(ToolId::Pickaxe), 0);
}

TEST(ToolControllerTest, RestoreRoundTripThroughProxy)
{
    ToolController tools;
    tools.restore(ToolId::Rod, 4, 10);
    EXPECT_EQ(tools.level(ToolId::Rod), 4);
    EXPECT_EQ(tools.durability(ToolId::Rod), 10);
    EXPECT_EQ(tools.max_durability(ToolId::Rod),
              Tool(ToolId::Rod).max_durability());
    // 越界值被夹取
    tools.restore(ToolId::Rod, -3, -1);
    EXPECT_EQ(tools.durability(ToolId::Rod), 0);
    EXPECT_GE(tools.level(ToolId::Rod), 1);
}