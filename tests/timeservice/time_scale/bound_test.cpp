/**
 * @file bound_test.cpp
 * @brief 时间倍率边界测试（tests/timeservice/time_scale/ 用例 TS-SC-011+）。
 *
 * DEF-103：set_time_scale 此前无边界校验，update() 内 double→int 转换
 * 对超大倍率（如 1e18）为 UB；负倍率依赖负累积回零。本用例锁定钳制语义：
 *   - 超大倍率钳制到上限，单帧推进量可预期且不产生未定义行为
 *   - 负倍率钳制为 0（静止），与 TS-SC-007 既有契约一致
 */
#include "time_service.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace
{
using mud::time::GameDateTime;

void push(mud::TimeService& svc, std::int64_t frames)
{
    for (std::int64_t i = 0; i < frames; ++i) svc.update();
}
} // namespace

TEST(ScaleBound, HugeScaleClampedNoOverflow) // TS-SC-011
{
    mud::TimeService svc;
    svc.set_time_scale(1e18);
    push(svc, 1);
    // 钳制上限 1e6 游戏秒/帧 → 单帧 floor(1e6/60)=16666 分钟
    EXPECT_EQ(svc.session_total(), 16666);
}

TEST(ScaleBound, NegativeScaleClampedToZero) // TS-SC-012
{
    mud::TimeService svc;
    svc.set_time_scale(-1e9);
    push(svc, 5);
    EXPECT_EQ(svc.session_total(), 0);
    EXPECT_EQ(svc.now(), GameDateTime{});
}

TEST(ScaleBound, UpperBoundStillWorks) // TS-SC-013
{
    mud::TimeService svc;
    svc.set_time_scale(1e6);
    push(svc, 1);
    EXPECT_EQ(svc.session_total(), 16666);
}
