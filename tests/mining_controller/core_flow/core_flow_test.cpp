/**
 * @file core_flow_test.cpp
 * @brief 采矿核心流程测试（方案 MIN-CF）：读条、状态机、产出结算、stop 结算。
 *
 * 被测对象：MiningHandler + MiningController + MiningState + MiningCalculator。
 * 时间推进：TimeService 默认倍率 60，update() 每帧 = 1 游戏分钟；默认工具 interval=1。
 * 数据来自 MUDGAME_DATA_DIR 下的 mining_layers.json / spawn_rates.json / ore.json。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_state.h"
#include "mining_types.h"
#include "time_service.h"

using namespace mud;

namespace
{

// 一套可复用控制器 + 会话处理器；ok_ctx 满足 level/照明/工具前置，便于聚焦流程。
struct MiningFixture
{
    mud::TimeService ts;
    Ore::OreData dummy_ore;      // 构造入参占位（控制器内部自加载真实 JSON）
    MiningController controller;
    MiningHandler handler;
    mining::MiningContext ok_ctx;

    MiningFixture()
        : controller(dummy_ore, ts), handler(controller)
    {
        ok_ctx.mining_level = 15;  // 满足所有层等级
        ok_ctx.has_torch = true;   // 满足火把层
        ok_ctx.has_lantern = true; // 满足灯笼层
        ok_ctx.tool.interval = 1;  // 每次轮询产出 1 份（每 1 游戏分钟）
    }

    void advance(std::int64_t frames)
    {
        for (std::int64_t i = 0; i < frames; ++i) ts.update();
    }
};

} // namespace

// MIN-CF-001 空闲态启动采矿：Idle → Mining，字段初始化
TEST(MiningCoreFlow, StartIdleEntersMining)
{
    MiningFixture f;
    ASSERT_FALSE(f.handler.is_mining());
    EXPECT_TRUE(f.handler.start(0, f.ok_ctx));
    EXPECT_TRUE(f.handler.is_mining());
    EXPECT_EQ(f.handler.status(), mining::MiningStatus::Mining);
    ASSERT_TRUE(f.handler.layer_id().has_value());
    EXPECT_EQ(*f.handler.layer_id(), 0);
}

// MIN-CF-002 采矿中重复启动（BUSY 拦截）：不覆盖会话
TEST(MiningCoreFlow, DoubleStartBlockedKeepsLayer)
{
    MiningFixture f;
    ASSERT_TRUE(f.handler.start(0, f.ok_ctx));
    EXPECT_FALSE(f.handler.start(2, f.ok_ctx)); // BUSY 下 start 返回 false
    ASSERT_TRUE(f.handler.layer_id().has_value());
    EXPECT_EQ(*f.handler.layer_id(), 0);        // 会话层未被覆盖
    EXPECT_TRUE(f.handler.is_mining());
}

// MIN-CF-004 读条到期恰好一次产出：elapsed==interval → 1 份，状态仍 Mining
TEST(MiningCoreFlow, PollAtIntervalProducesOne)
{
    MiningFixture f;
    ASSERT_TRUE(f.handler.start(0, f.ok_ctx));
    f.advance(1); // 1 游戏分钟 = 1 interval
    const auto results = f.handler.poll(f.ok_ctx);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].ore_id.empty());
    EXPECT_EQ(results[0].quantity, 1u);
    EXPECT_GT(results[0].experience, 0u);
    EXPECT_TRUE(f.handler.is_mining()); // 未到期不结束会话
}

// MIN-CF-005 未到下一次 interval 不产出：elapsed==0 → 空
TEST(MiningCoreFlow, PollBeforeIntervalEmpty)
{
    MiningFixture f;
    ASSERT_TRUE(f.handler.start(0, f.ok_ctx));
    const auto results = f.handler.poll(f.ok_ctx); // 时间未推进，elapsed=0
    EXPECT_TRUE(results.empty());
    EXPECT_TRUE(f.handler.is_mining());
}

// MIN-CF-006 跨多个 interval 一次结算多份：elapsed=7, interval=3 → 2 份；剩余进度保留
TEST(MiningCoreFlow, PollAcrossMultipleIntervals)
{
    MiningFixture f;
    f.ok_ctx.tool.interval = 3; // 每 3 游戏分钟 1 次
    ASSERT_TRUE(f.handler.start(0, f.ok_ctx));
    f.advance(7); // elapsed=7 → count=7/3=2
    const auto results = f.handler.poll(f.ok_ctx);
    ASSERT_EQ(results.size(), 2u);
    EXPECT_TRUE(f.handler.is_mining());
    // 余下 1 分钟不足 interval，再次 poll（不推进）应为空
    EXPECT_TRUE(f.handler.poll(f.ok_ctx).empty());
}

// MIN-CF-007 采矿中停止：结算未提取产出并复位 Idle
TEST(MiningCoreFlow, StopSettlesAndResetsIdle)
{
    MiningFixture f;
    ASSERT_TRUE(f.handler.start(0, f.ok_ctx));
    f.advance(3); // elapsed=3, interval=1 → 3 份
    const auto settled = f.handler.stop(f.ok_ctx);
    ASSERT_EQ(settled.size(), 3u);
    EXPECT_FALSE(f.handler.is_mining());
    EXPECT_EQ(f.handler.status(), mining::MiningStatus::Idle);
    EXPECT_FALSE(f.handler.layer_id().has_value());
}

// MIN-CF-008 停止后恢复可再次采矿；Idle 可再入，无残留脏状态
TEST(MiningCoreFlow, RestartAfterStop)
{
    MiningFixture f;
    ASSERT_TRUE(f.handler.start(0, f.ok_ctx));
    f.advance(2); // 累计 2 份产出后再停止
    ASSERT_FALSE(f.handler.stop(f.ok_ctx).empty());
    EXPECT_TRUE(f.handler.start(2, f.ok_ctx)); // 换层可再入
    EXPECT_TRUE(f.handler.is_mining());
    ASSERT_TRUE(f.handler.layer_id().has_value());
    EXPECT_EQ(*f.handler.layer_id(), 2);
}

// MIN-CF-009 状态查询只读，不产生副作用
TEST(MiningCoreFlow, StatusQueryReadOnly)
{
    MiningFixture f;
    EXPECT_EQ(f.handler.status(), mining::MiningStatus::Idle);
    EXPECT_FALSE(f.handler.layer_id().has_value());
    ASSERT_TRUE(f.handler.start(0, f.ok_ctx));
    f.advance(2);
    // 查询本身不结算、不改状态
    const auto before = f.handler.status();
    EXPECT_EQ(f.handler.status(), before);
    ASSERT_TRUE(f.handler.layer_id().has_value());
    EXPECT_EQ(*f.handler.layer_id(), 0);
    // 时间已过而未 poll，产出仍可补捞（未被查询吞掉）
    EXPECT_EQ(f.handler.poll(f.ok_ctx).size(), 2u);
}

// MIN-CF-010 未开采即 poll / stop：空会话无副作用
TEST(MiningCoreFlow, PollAndStopWhenIdleAreNoops)
{
    MiningFixture f;
    EXPECT_TRUE(f.handler.poll(f.ok_ctx).empty());
    EXPECT_TRUE(f.handler.stop(f.ok_ctx).empty());
    EXPECT_FALSE(f.handler.is_mining());
}

// MIN-CF-011 前台即时采矿：produce_once 无需启动会话/推进时间即可产出一次
TEST(MiningCoreFlow, ProduceOnceReturnsSingleResult)
{
    MiningFixture f;
    const auto result = f.controller.produce_once(0, f.ok_ctx);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->ore_id.empty());
    EXPECT_EQ(result->quantity, 1u);
    EXPECT_GT(result->experience, 0u);
    // 不产生任何会话副作用
    EXPECT_FALSE(f.handler.is_mining());
}

// MIN-CF-012 非法层 produce_once 返回空
TEST(MiningCoreFlow, ProduceOnceOnInvalidLayerEmpty)
{
    MiningFixture f;
    EXPECT_FALSE(f.controller.produce_once(99, f.ok_ctx).has_value());
}

// MIN-CF-013 层条件预检 can_enter：合法层通过，越界层拒绝，不启动会话
TEST(MiningCoreFlow, CanEnterChecksLayerWithoutSession)
{
    MiningFixture f;
    EXPECT_TRUE(f.controller.can_enter(0, f.ok_ctx));
    EXPECT_FALSE(f.controller.can_enter(99, f.ok_ctx));
    EXPECT_FALSE(f.handler.is_mining()); // 预检无副作用
}