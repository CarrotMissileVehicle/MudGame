/**
 * @file clock_anomaly_test.cpp
 * @brief 系统时钟前跳/后跳测试（tests/timeservice/clock_anomaly/ 用例 TS-CA-*）。
 *
 * 聚焦 NTP 类时钟跳变对时间管理器的影响：
 *  - 前跳：跨过到期时刻导致事件错过（当前窗口语义，验证确定行为 + 不崩溃）。
 *  - 后跳：session_total 非单调（R2 当前为已知缺口；单调保护未实现用例被跳过登记）。
 *  - set_time 前跳后挂起定时不复活。
 */
#include "time_service.h"

#include <gtest/gtest.h>

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

TEST(ClockAnomaly, ForwardJumpSkipsPendingTimer) // TS-CA-001 前跳越过到期不触发
{
    mud::TimeService svc; // 00:00
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 3), [&] { ++fired; });
    svc.set_time(dt(0, 1, 1, 0, 30)); // 前跳，未走 update 窗口
    push(svc, 5);
    EXPECT_EQ(fired, 0); // 当前窗口语义下错过；不崩溃、可观测
}

TEST(ClockAnomaly, BackwardJumpIsReflectedAndAdvances) // TS-CA-002 后跳后按新基准推进
{
    mud::TimeService svc(dt(0, 1, 1, 0, 10)); // 00:10
    push(svc, 1);                             // 00:11
    svc.set_time(dt(0, 1, 1, 0, 5));          // 后跳到 00:05
    EXPECT_EQ(svc.session_total(), 5);        // 反映 set_time 后时刻
    push(svc, 1);                             // 从 00:05 继续
    EXPECT_EQ(svc.session_total(), 6);
}

TEST(ClockAnomaly, BackwardJumpDoesNotRefireFiredTimer) // TS-CA-003 已触发过不因倒流复活
{
    mud::TimeService svc; // 00:00
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 3), [&] { ++fired; });
    push(svc, 4); // 触发一次
    EXPECT_EQ(fired, 1);
    svc.set_time(dt(0, 1, 1, 0, 0)); // 倒流回 00:00
    push(svc, 10);
    EXPECT_EQ(fired, 1); // 一次性已移除，不重复触发
}

TEST(ClockAnomaly, MonotonicGuardNotImplemented_Gap) // TS-CA-002b R2 单调保护缺口登记
{
    mud::TimeService svc(dt(0, 1, 1, 0, 10));
    push(svc, 5);
    svc.set_time(dt(0, 1, 1, 0, 1)); // 后跳
    GTEST_SKIP() << "R2 已知缺口：set_time 无条件覆盖、无单调保护；实现单调时钟后启用本用例";
    EXPECT_TRUE(svc.session_total() >= 15); // 目标：任何后跳不得使会话总量回退
}

TEST(ClockAnomaly, ForwardJumpThenBackwardNoOverflow) // 复合跳变不崩溃
{
    mud::TimeService svc;
    svc.set_time(dt(2020, 6, 1, 12, 0));
    push(svc, 5);
    svc.set_time(dt(2000, 1, 1, 0, 0));
    push(svc, 5);
    const auto s = svc.session_total();
    EXPECT_GE(s, 0); // 正常返回、不回退到负
}