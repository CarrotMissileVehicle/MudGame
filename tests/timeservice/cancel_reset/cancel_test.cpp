/**
 * @file cancel_test.cpp
 * @brief 定时器取消/重置/玩家打断测试（tests/timeservice/cancel_reset/ 用例 TS-CR-*）。
 *
 * 重点验证防「幽灵结算」：取消后同帧到期不得触发；取消不存在令牌无害；幂等；
 * set_time 重置对挂起定时器的影响；玩家打断读条后关联定时器不再结算。
 */
#include "time_service.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace
{
using mud::time::GameDateTime;

GameDateTime dt(int y, unsigned m, unsigned d, unsigned h = 0, unsigned min = 0)
{
    return GameDateTime{y, m, d, h, min};
}

void push(mud::TimeService& svc, std::int64_t frames)
{
    for (std::int64_t i = 0; i < frames; ++i) svc.update();
}
} // namespace

// 采矿结算副作用计数：验证取消后不产生「幽灵结算」。
TEST(Cancel, CancelledTimerDoesNotFire) // TS-CR-001 取消后到期不触发
{
    mud::TimeService svc; // 00:00
    int settlement = 0;
    const auto tok = svc.schedule_time(dt(0, 1, 1, 0, 3), [&] { ++settlement; });
    svc.cancel(tok);
    push(svc, 10); // 跨过 due 00:03
    EXPECT_EQ(settlement, 0); // 幽灵结算判定 → 通过
}

TEST(Cancel, CancelledIntervalStopsForever) // TS-CR-002 取消周期定时永久停止
{
    mud::TimeService svc;
    int fired = 0;
    const auto tok = svc.schedule_interval(5, [&] { ++fired; });
    push(svc, 5);
    EXPECT_EQ(fired, 1);
    svc.cancel(tok);
    push(svc, 50); // 多期后仍不触发
    EXPECT_EQ(fired, 1);
}

TEST(Cancel, CancelUnknownTokenIsHarmless) // TS-CR-003/004 无效/重复取消无害
{
    mud::TimeService svc;
    int a = 0, b = 0;
    const auto tok_a = svc.schedule_time(dt(0, 1, 1, 0, 2), [&] { ++a; });
    const auto tok_b = svc.schedule_time(dt(0, 1, 1, 0, 2), [&] { ++b; });
    svc.cancel(99999); // 不存在
    svc.cancel(99999); // 再取消仍无害
    svc.cancel(tok_a);
    svc.cancel(tok_a); // 幂等
    svc.cancel(tok_b);
    push(svc, 5);
    EXPECT_EQ(a, 0);
    EXPECT_EQ(b, 0); // 两条目均应被取消
}

TEST(Cancel, CancelThenSetTimeSkipsStillNotFire) // TS-CR-005 取消后重置不复活
{
    mud::TimeService svc;
    int fired = 0;
    const auto tok = svc.schedule_time(dt(0, 1, 1, 0, 30), [&] { ++fired; });
    svc.cancel(tok);
    svc.set_time(dt(0, 1, 1, 1, 0)); // 直接越过 due
    push(svc, 1);
    EXPECT_EQ(fired, 0);
}

TEST(Cancel, ResetLeavesUncancelledTimers) // TS-CR-006 重置不清除未取消条目
{
    mud::TimeService svc; // 00:00
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 5), [&] { ++fired; });
    svc.set_time(dt(0, 1, 1, 0, 1)); // 重置到 00:01
    push(svc, 4);                     // 00:01 → 00:05
    EXPECT_EQ(fired, 1);              // 未取消条目正常到期
}

// 玩家打断采矿读条 → 取消定时 → 不得产出「幽灵结算」。
TEST(Cancel, PlayerInterruptNoGhostSettlement) // TS-CR-007 打断读条防幽灵结算
{
    mud::TimeService svc; // 00:00；采矿读条=周期结算 5 分钟
    int ore = 0;
    const auto mining = svc.schedule_interval(5, [&] { ++ore; });
    // 玩家在第 3 分钟打断
    push(svc, 3);
    svc.cancel(mining);
    push(svc, 30); // 越过后 6 个结算周期
    EXPECT_EQ(ore, 0); // 打断后不得再结算
}

TEST(Cancel, RestartAfterInterruptNoCrosstalk) // TS-CR-008 重开不串扰
{
    mud::TimeService svc;
    int o1 = 0, o2 = 0;
    const auto t1 = svc.schedule_time(dt(0, 1, 1, 0, 4), [&] { o1 = 1; });
    svc.cancel(t1); // 第一次被打断
    svc.schedule_time(dt(0, 1, 1, 0, 6), [&] { o2 = 1; }); // 重新开始第二次
    push(svc, 8);
    EXPECT_EQ(o1, 0); // 旧操作不复活
    EXPECT_EQ(o2, 1); // 新操作正常完成
}

TEST(Cancel, CancelBeforeDueSameFrame) // TS-CR-009 取消优先于到期
{
    mud::TimeService svc;
    int fired = 0;
    const auto tok = svc.schedule_time(dt(0, 1, 1, 0, 1), [&] { ++fired; });
    svc.cancel(tok); // 到期帧前取消
    svc.update();
    EXPECT_EQ(fired, 0);
}

TEST(Cancel, SetTimeForwardSkipsAndDoesNotRefire) // TS-CR-010 前跳使过期定时不复活
{
    mud::TimeService svc; // 00:00
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 3), [&] { ++fired; });
    svc.set_time(dt(0, 1, 1, 0, 30)); // 前跳跨过 00:03（未走过 update 窗口）
    push(svc, 5);
    EXPECT_EQ(fired, 0); // 当前实现窗口语义：不触发（不崩溃、可观测）
}

TEST(Cancel, CancelManyLeavesNoResidue) // TS-CR-011 批量取消
{
    mud::TimeService svc;
    std::size_t count = 0;
    std::vector<std::size_t> toks;
    toks.reserve(1000);
    for (std::size_t i = 0; i < 1000; ++i)
    {
        toks.push_back(svc.schedule_interval(1, [&] { ++count; }));
    }
    for (auto t : toks) svc.cancel(t);
    push(svc, 10);
    EXPECT_EQ(count, 0); // 全部取消后无一次触发
}