/**
 * @file tick_test.cpp
 * @brief Tick/心跳推进测试（tests/timeservice/tick_engine/ 用例 TS-TE-*）。
 *
 * 验证帧驱动 update() 的推进、亚分钟累计进位、无磁越帧不推进/不触发、
 * 大倍率单帧跨多分钟、0 倍率静止与长跑单调性。
 */
#include "time_service.h"

#include <gtest/gtest.h>

#include <cstdint>

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

TEST(Tick, DefaultScaleAdvancesOneMinutePerFrame) // TS-TE-001
{
    mud::TimeService svc; // 默认倍率 60
    svc.update();
    EXPECT_EQ(svc.now(), dt(0, 1, 1, 0, 1));
    EXPECT_EQ(svc.session_total(), 1);
}

TEST(Tick, DailyAccumulationAtScale60) // TS-TE-002
{
    mud::TimeService svc;
    push(svc, 1440); // 1440 分钟 = 1 天
    EXPECT_EQ(svc.now(), dt(0, 1, 2, 0, 0));
    EXPECT_EQ(svc.session_total(), 1440);
}

TEST(Tick, SubMinuteNotYetMinute) // TS-TE-003 余数未满 60 不推进
{
    mud::TimeService svc;
    svc.set_time_scale(1.0);
    push(svc, 59);
    EXPECT_EQ(svc.session_total(), 0);  // 59 游戏秒 < 1 分钟
    EXPECT_EQ(svc.now(), dt(0, 1, 1, 0, 0));
}

TEST(Tick, SubMinuteCarryAt60Seconds) // TS-TE-004 第 60 帧进位
{
    mud::TimeService svc;
    svc.set_time_scale(1.0);
    push(svc, 60);
    EXPECT_EQ(svc.session_total(), 1);
    EXPECT_EQ(svc.now(), dt(0, 1, 1, 0, 1));
}

TEST(Tick, CrossMidnightFrom2359) // TS-TE-005 跨日进位
{
    mud::TimeService svc(dt(0, 1, 1, 23, 59));
    svc.update();
    EXPECT_EQ(svc.now(), dt(0, 1, 2, 0, 0));
}

TEST(Tick, SingleFrameLargeScaleCrossesMultipleMinutes) // TS-TE-006
{
    mud::TimeService svc;
    svc.set_time_scale(3600.0); // 1 实秒 → 3600 游戏秒 = 60 分钟
    svc.update();
    EXPECT_EQ(svc.now(), dt(0, 1, 1, 1, 0));
    EXPECT_EQ(svc.session_total(), 60);
}

TEST(Tick, FractionalScaleCarryConserved) // TS-TE-007 非整数倍率长跑进位守恒
{
    mud::TimeService svc;
    svc.set_time_scale(30.5);
    push(svc, 120); // 30.5*120=3660 游戏秒 → 61 分钟
    EXPECT_EQ(svc.session_total(), 61);
}

TEST(Tick, ZeroScaleIsStationary) // TS-TE-008 0 倍率时间静止
{
    mud::TimeService svc;
    svc.set_time_scale(0.0);
    push(svc, 100);
    EXPECT_EQ(svc.session_total(), 0);
    EXPECT_EQ(svc.now(), dt(0, 1, 1, 0, 0));
}

TEST(Tick, NoCrossNoCallbackFire) // TS-TE-009 无越帧不触发
{
    mud::TimeService svc;
    svc.set_time_scale(1.0);
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 5, 0), [&] { ++fired; });
    push(svc, 59); // 0 游戏分钟跨越，未到 5:00 更不会触发
    EXPECT_EQ(fired, 0);
}

TEST(Tick, HeartbeatLongRunMonotonic) // TS-TE-010 长跑单调一致
{
    mud::TimeService svc;
    constexpr std::int64_t N = 10000;
    std::int64_t last = -1;
    for (std::int64_t i = 0; i < N; ++i)
    {
        svc.update();
        EXPECT_GT(svc.session_total(), last); // 严格递增
        last = svc.session_total();
    }
    EXPECT_EQ(last, N);
}