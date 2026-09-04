/**
 * @file boundary_test.cpp
 * @brief 采矿边界与异常场景测试（方案 MIN-BD）：越界层号、双击、会话边界健壮性。
 *
 * 关注：任何非法/畸形输入不导致崩溃、无未捕获异常、无误结算。
 * 被测对象：MiningHandler / MiningController 的输入边界兜底。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_types.h"
#include "time_service.h"

#include <limits>

using namespace mud;

namespace
{

struct BoundFixture
{
    mud::TimeService ts;
    Ore::OreData dummy_ore;
    MiningController controller;
    MiningHandler handler;
    mining::MiningContext ctx;

    BoundFixture() : controller(dummy_ore, ts), handler(controller)
    {
        ctx.mining_level = 15;
        ctx.has_torch = true;
        ctx.has_lantern = true;
        ctx.tool.interval = 1;
    }
};

} // namespace

// MIN-BD-003 越界层号：≥5 及 SIZE_MAX 均被拒绝，且不进入采矿
TEST(MiningBoundary, OutOfRangeLayerRejected)
{
    BoundFixture f;
    EXPECT_FALSE(f.handler.start(5, f.ctx));
    EXPECT_FALSE(f.handler.start(99, f.ctx));
    EXPECT_FALSE(f.handler.start(std::numeric_limits<std::size_t>::max(), f.ctx));
    EXPECT_FALSE(f.handler.is_mining());
    EXPECT_FALSE(f.handler.layer_id().has_value());
}

// MIN-BD-002 重复点击同一层 start：BUSY 下幂等拒绝，会话不被破坏
TEST(MiningBoundary, DoubleTapStartIsIdempotent)
{
    BoundFixture f;
    ASSERT_TRUE(f.handler.start(0, f.ctx));
    EXPECT_FALSE(f.handler.start(0, f.ctx)); // 双击同一层同样被拒
    ASSERT_TRUE(f.handler.layer_id().has_value());
    EXPECT_EQ(*f.handler.layer_id(), 0);
    EXPECT_TRUE(f.handler.is_mining());
    // 会话仍可正常产出
    f.ts.update();
    EXPECT_EQ(f.handler.poll(f.ctx).size(), 1u);
}

// MIN-BD-005 取消采矿（stop 主动结束）后清账可再入，无脏状态
TEST(MiningBoundary, CancelledSessionFullyResets)
{
    BoundFixture f;
    ASSERT_TRUE(f.handler.start(1, f.ctx));
    f.ts.update();
    EXPECT_FALSE(f.handler.stop(f.ctx).empty()); // 结算并复位
    EXPECT_FALSE(f.handler.is_mining());
    EXPECT_FALSE(f.handler.layer_id().has_value());
    // 复位后重新进入不同层成功
    EXPECT_TRUE(f.handler.start(4, f.ctx));
    EXPECT_TRUE(f.handler.is_mining());
    EXPECT_EQ(*f.handler.layer_id(), 4);
}

// MIN-BD-006 层号极端值不异常、不崩溃（非法输入兜底）
TEST(MiningBoundary, ExtremeLayerIdsDoNotCrash)
{
    BoundFixture f;
    for (std::size_t bad : {std::size_t{6}, std::size_t{1000},
                            std::size_t{0xFFFFFFFF}, std::size_t{0xFFFFFFFFFFFFFFFF}})
    {
        EXPECT_FALSE(f.handler.start(bad, f.ctx));
    }
    EXPECT_FALSE(f.handler.is_mining());
}

// ---- 未落地能力的边界契约（速率限制、离线、畸形指令由命令层/数据层承载） ----

// MIN-BD-001 外挂高频请求限流（未实现，标注跳过）
TEST(MiningBoundary, RateLimitNotImplemented)
{
    GTEST_SKIP() << "命令层速率限制/去重尚未实现（方案 MIN-BD-001）";
}

// MIN-BD-007 工具损坏下轮询无副作用（未实现，标注跳过）
TEST(MiningBoundary, BrokenToolPollNotImplemented)
{
    GTEST_SKIP() << "矿镐耐久与损坏态尚未实现（方案 MIN-BD-007）";
}