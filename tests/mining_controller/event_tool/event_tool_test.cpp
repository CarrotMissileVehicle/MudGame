/**
 * @file event_tool_test.cpp
 * @brief 事件/工具接入采矿测试：判定规则、工具机制、控制器集成。
 *
 * 覆盖：
 *   - mud::event::EventSystem 边界规则（宝箱 >=93 / 塌方 <5）。
 *   - mud::tool::Tool 使用/耐久/等级加成。
 *   - mud::tool::ToolController 使用与耐久查询。
 *   - 将工具接入 MiningController：矿镐每次产出扣 1，耐久耗尽即中断并结束会话。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_state.h"
#include "mining_types.h"
#include "time_service.h"

#include "event.h"
#include "tool.h"
#include "tool_Controller.h"

using namespace mud;

namespace
{

// 可复用的采矿测试上下文：满足全部层前置。
mining::MiningContext ok_ctx()
{
    mining::MiningContext ctx;
    ctx.mining_level = 15;
    ctx.has_torch   = true;
    ctx.has_lantern = true;
    ctx.tool.interval = 1; // 每帧 = 1 游戏分钟 = 1 次产出
    return ctx;
}

} // namespace

// ---- 事件判定边界（确定性）----
TEST(MiningEventTool, EventCaveAndChestBoundaries)
{
    mud::event::EventSystem ev;
    bool chest = true, cave = true;

    ev.roll_mining_event(chest, cave, 1);  // 塌方区起点
    EXPECT_FALSE(chest); EXPECT_TRUE(cave);

    ev.roll_mining_event(chest, cave, 4);  // 塌方区终点
    EXPECT_FALSE(chest); EXPECT_TRUE(cave);

    ev.roll_mining_event(chest, cave, 5);  // 塌方区分界外
    EXPECT_FALSE(chest); EXPECT_FALSE(cave);

    ev.roll_mining_event(chest, cave, 92); // 宝箱区前
    EXPECT_FALSE(chest); EXPECT_FALSE(cave);

    ev.roll_mining_event(chest, cave, 93); // 宝箱区起点
    EXPECT_TRUE(chest); EXPECT_FALSE(cave);

    ev.roll_mining_event(chest, cave, 100);// 宝箱区终点
    EXPECT_TRUE(chest); EXPECT_FALSE(cave);
}

// ---- 工具机制（确定性）----
TEST(MiningEventTool, ToolUseConsumesDurabilityAndLevelBonus)
{
    mud::tool::Tool pick(mud::tool::ToolId::Pickaxe);
    EXPECT_EQ(pick.max_durability(), 20);
    EXPECT_EQ(pick.durability(), 20);
    EXPECT_FALSE(pick.is_broken());
    EXPECT_EQ(pick.level(), 1);
    EXPECT_EQ(pick.level_bonus(), 0) << "等级 1 无加成";

    for (int i = 1; i <= 20; ++i)
    {
        EXPECT_TRUE(pick.use());
        EXPECT_EQ(pick.durability(), 20 - i);
    }
    EXPECT_TRUE(pick.is_broken()) << "耐久尽后损坏";
    EXPECT_FALSE(pick.use()) << "损坏后不可用";

    pick.repair_fully();
    EXPECT_EQ(pick.durability(), 20);
    EXPECT_FALSE(pick.is_broken());

    EXPECT_TRUE(pick.upgrade());
    EXPECT_EQ(pick.level(), 2);
    EXPECT_EQ(pick.level_bonus(), 1) << "等级 2 → 加成 1";
}

TEST(MiningEventTool, ToolControllerDrivesPickaxe)
{
    mud::tool::ToolController ctrl;
    EXPECT_EQ(ctrl.durability(mud::tool::ToolId::Pickaxe), 20);
    EXPECT_TRUE(ctrl.use_tool(mud::tool::ToolId::Pickaxe)); // 耐久 20→19

    for (int i = 0; i < 18; ++i) // 累计 19 次成功使用 → 耐久 1
        ASSERT_TRUE(ctrl.use_tool(mud::tool::ToolId::Pickaxe));
    EXPECT_EQ(ctrl.durability(mud::tool::ToolId::Pickaxe), 1);
    EXPECT_FALSE(ctrl.is_broken(mud::tool::ToolId::Pickaxe));

    ASSERT_TRUE(ctrl.use_tool(mud::tool::ToolId::Pickaxe)); // 第 20 次 → 耐久 0
    EXPECT_EQ(ctrl.durability(mud::tool::ToolId::Pickaxe), 0);
    EXPECT_TRUE(ctrl.is_broken(mud::tool::ToolId::Pickaxe));
    EXPECT_FALSE(ctrl.use_tool(mud::tool::ToolId::Pickaxe));
}

// ---- 集成：矿镐损坏中断采矿会话（确定性，events 未注入）----
TEST(MiningEventTool, PickaxeBreakInterruptsMining)
{
    mud::TimeService ts;
    Ore::OreData ore;
    mud::tool::ToolController tools;
    MiningController controller(ore, ts, nullptr, &tools);
    MiningHandler handler(controller);
    const auto ctx = ok_ctx();

    ASSERT_TRUE(handler.start(0, ctx));

    // 前 20 次成功产出各扣 1 耐久，会话保持 Mining。
    for (int i = 1; i <= 20; ++i)
    {
        ts.update();
        const auto r = handler.poll(ctx);
        ASSERT_EQ(r.size(), 1u) << "第 " << i << " 次产出未发生";
        EXPECT_FALSE(r[0].ore_id.empty());
        EXPECT_EQ(r[0].quantity, 1u);
        EXPECT_GT(r[0].experience, 0u);
        EXPECT_EQ(tools.durability(mud::tool::ToolId::Pickaxe), 20 - i);
    }
    EXPECT_TRUE(tools.is_broken(mud::tool::ToolId::Pickaxe));
    EXPECT_TRUE(handler.is_mining()) << "未损坏前会话持续";

    // 第 21 次：矿镐已损坏 → 中断并结束会话。
    ts.update();
    const auto r = handler.poll(ctx);
    EXPECT_TRUE(r.empty());
    EXPECT_FALSE(handler.is_mining());
}

// ---- 集成：事件接入不破坏产出形态（events 注入，容许随机塌方/宝箱）----
TEST(MiningEventTool, EventWiringKeepsResultShape)
{
    mud::TimeService ts;
    Ore::OreData ore;
    mud::tool::ToolController tools;
    mud::event::EventSystem ev;
    MiningController controller(ore, ts, &ev, &tools);
    MiningHandler handler(controller);
    const auto ctx = ok_ctx();

    ASSERT_TRUE(handler.start(0, ctx));
    ts.update();
    const auto r = handler.poll(ctx);
    // 塌方 → 空且会话结束；否则为一次(1 或 2 数量)产出。
    ASSERT_LE(r.size(), 1u);
    for (const auto& res : r)
    {
        EXPECT_FALSE(res.ore_id.empty());
        EXPECT_GE(res.quantity, 1u);
        EXPECT_LE(res.quantity, 2u);
        EXPECT_GT(res.experience, 0u);
    }
    EXPECT_LE(tools.durability(mud::tool::ToolId::Pickaxe), 20);
}