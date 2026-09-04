/**
 * @file scale_test.cpp
 * @brief 时间倍率换算测试（tests/timeservice/time_scale/ 用例 TS-SC-*）。
 *
 * 验证时间流速倍率 set_time_scale() 的换算正确性、非整倍率、中途变速余数保留、
 * 0 倍率静止与恢复、负倍率健壮性。
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

TEST(Scale, DefaultRate60PerFrame) // TS-SC-001
{
    mud::TimeService svc;
    push(svc, 1);
    EXPECT_EQ(svc.session_total(), 1); // 60 游戏秒=1 分钟
}

TEST(Scale, Accelerated10x) // TS-SC-002
{
    mud::TimeService svc;
    svc.set_time_scale(600.0);
    push(svc, 1);
    EXPECT_EQ(svc.now(), dt(0, 1, 1, 0, 10)); // 600 游戏秒=10 分钟
    EXPECT_EQ(svc.session_total(), 10);
}

TEST(Scale, SlowMotionOneRealSecPerGameSec) // TS-SC-003
{
    mud::TimeService svc;
    svc.set_time_scale(1.0);
    push(svc, 180);
    EXPECT_EQ(svc.session_total(), 3); // 180 游戏秒=3 分钟
}

TEST(Scale, OneThirdRateLongRun) // TS-SC-004 非整倍率
{
    mud::TimeService svc;
    svc.set_time_scale(1.0 / 3.0);
    push(svc, 600); // 600*(1/3)=200 游戏秒 → 3 分钟（远离分钟边界，稳妥）
    EXPECT_EQ(svc.session_total(), 3);
}

TEST(Scale, MidwayAccelerationCompletes) // TS-SC-005 中途加速补齐
{
    mud::TimeService svc;
    svc.set_time_scale(1.0);
    push(svc, 30); // 30 游戏秒
    svc.set_time_scale(60.0);
    push(svc, 1); // +60 游戏秒 = 90 → 1 分钟 (余 30)
    EXPECT_EQ(svc.session_total(), 1);
}

TEST(Scale, MidwayDecelerationKeepsRemainder) // TS-SC-006 中途减速余数保留
{
    mud::TimeService svc;
    svc.set_time_scale(60.0);
    push(svc, 1); // 1 分钟，余 0
    svc.set_time_scale(10.0);
    push(svc, 6); // +60 游戏秒 → 1 分钟
    EXPECT_EQ(svc.session_total(), 2);
}

TEST(Scale, NegativeRateIsRejectedNoCrash) // TS-SC-007 负倍率健壮性
{
    mud::TimeService svc;
    svc.set_time_scale(-1.0);
    svc.update();
    svc.update();
    EXPECT_EQ(svc.session_total(), 0);   // 无推进
    EXPECT_EQ(svc.now(), dt(0, 1, 1, 0, 0)); // 时间未变、不崩溃
}

TEST(Scale, ZeroThenRevive) // TS-SC-009 0 倍率静止后可恢复
{
    mud::TimeService svc;
    svc.set_time_scale(0.0);
    push(svc, 100);
    svc.set_time_scale(60.0);
    push(svc, 1);
    EXPECT_EQ(svc.session_total(), 1); // 静止段不计入
}

TEST(Scale, RateAffectsScheduledDue) // TS-SC-010 变速影响到期所需帧数
{
    mud::TimeService svc; // 初始 00:00
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 30), [&] { ++fired; }); // 30 分钟后
    svc.set_time_scale(600.0); // 1 帧 10 分钟
    push(svc, 3);              // 3 帧=30 分钟 → 到期
    EXPECT_EQ(fired, 1);
    push(svc, 1);
    EXPECT_EQ(fired, 1); // 一次性不重复
}