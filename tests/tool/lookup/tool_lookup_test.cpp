// DEF-006 回归：ToolController 按 ToolId 查找工具（map 化改造后的行为锁）。
// 配置基准：锄头(50耐久) / 鱼竿(40) / 矿镐(20)，每次使用扣 1 耐久。
#include <gtest/gtest.h>

#include "tool_controller.h"

// 各工具基础属性按 ToolId 正确映射（不因枚举/数组错配而张冠李戴）
TEST(ToolLookup, AttributesMappedByToolId)
{
    mud::tool::ToolController tools;

    EXPECT_EQ(tools.name(mud::tool::ToolId::Hoe), "锄头");
    EXPECT_EQ(tools.name(mud::tool::ToolId::Rod), "鱼竿");
    EXPECT_EQ(tools.name(mud::tool::ToolId::Pickaxe), "矿镐");

    EXPECT_EQ(tools.max_durability(mud::tool::ToolId::Hoe), 50);
    EXPECT_EQ(tools.max_durability(mud::tool::ToolId::Rod), 40);
    EXPECT_EQ(tools.max_durability(mud::tool::ToolId::Pickaxe), 20);
}

// 使用工具扣减耐久；三件工具的耐久互相独立
TEST(ToolLookup, UseToolDrainsOwnDurabilityOnly)
{
    mud::tool::ToolController tools;

    ASSERT_TRUE(tools.use_tool(mud::tool::ToolId::Pickaxe));
    EXPECT_EQ(tools.durability(mud::tool::ToolId::Pickaxe), 19);
    EXPECT_EQ(tools.durability(mud::tool::ToolId::Hoe), 50);   // 其他工具不受影响
    EXPECT_EQ(tools.durability(mud::tool::ToolId::Rod), 40);
}

// 耐久耗尽后 is_broken / is_full 语义正确
TEST(ToolLookup, BrokenWhenDurabilityBelowPerUse)
{
    mud::tool::ToolController tools;

    for (int i = 0; i < 20; ++i)
        tools.use_tool(mud::tool::ToolId::Pickaxe);            // 矿镐 20 耐久用尽
    EXPECT_TRUE(tools.is_broken(mud::tool::ToolId::Pickaxe));
    EXPECT_FALSE(tools.use_tool(mud::tool::ToolId::Pickaxe));  // 损坏后不可再用

    tools.repair_full(mud::tool::ToolId::Pickaxe);
    EXPECT_EQ(tools.durability(mud::tool::ToolId::Pickaxe), 20);
    EXPECT_TRUE(tools.is_full(mud::tool::ToolId::Pickaxe));
    EXPECT_FALSE(tools.is_broken(mud::tool::ToolId::Pickaxe));
}

// 修复成本：按损耗折算，满耐久为 0
TEST(ToolLookup, GoldRepairCostScalesWithWear)
{
    mud::tool::ToolController tools;

    EXPECT_EQ(tools.gold_repair_cost(mud::tool::ToolId::Hoe), 0);   // 满耐久免费

    tools.use_tool(mud::tool::ToolId::Hoe);                          // 损耗 1/50
    EXPECT_GT(tools.gold_repair_cost(mud::tool::ToolId::Hoe), 0);
    EXPECT_LE(tools.gold_repair_cost(mud::tool::ToolId::Hoe), 50);   // 基准 50 按比例
}

// 读档还原：等级与耐久写入正确
TEST(ToolLookup, RestoreSetsLevelAndDurability)
{
    mud::tool::ToolController tools;

    tools.restore(mud::tool::ToolId::Rod, 3, 17);
    EXPECT_EQ(tools.level(mud::tool::ToolId::Rod), 3);
    EXPECT_EQ(tools.level_bonus(mud::tool::ToolId::Rod), 2);
    EXPECT_EQ(tools.durability(mud::tool::ToolId::Rod), 17);
}

// 修复所需矿石信息按工具正确返回
TEST(ToolLookup, RepairOreInfoMappedByToolId)
{
    mud::tool::ToolController tools;

    EXPECT_EQ(tools.repair_ore(mud::tool::ToolId::Hoe), "黯铁矿");
    EXPECT_EQ(tools.repair_ore(mud::tool::ToolId::Rod), "星纹银");
    EXPECT_EQ(tools.repair_ore(mud::tool::ToolId::Pickaxe), "黯铁矿");
    EXPECT_EQ(tools.repair_ore_count(mud::tool::ToolId::Hoe), 2);
}
