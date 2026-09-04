/**
 * @file precision_test.cpp
 * @brief 长时间运行精度漂移测试（tests/timeservice/precision/ 用例 TS-PR-*）。
 *
 * 验证长跑后游戏分钟与按倍率期望一致、漂移有界不随帧数线性发散、时间推进单调。
 * 期望源用独立累加公式 S=Σ(帧数×倍率)，分钟 E=floor(S/60)，漂移 Δ=|session_total-E|。
 */
#include "time_service.h"

#include <gtest/gtest.h>

#include <cmath>
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

TEST(Precision, IntegerScaleExactLongRun) // TS-PR-001 整倍率长跑 0 漂移
{
    mud::TimeService svc;
    svc.set_time_scale(60.0);
    constexpr std::int64_t N = 100000;
    push(svc, N);
    EXPECT_EQ(svc.session_total(), N); // Δ=0
}

TEST(Precision, FractionalScaleDriftBounded) // TS-PR-002 非整倍率漂移有界
{
    mud::TimeService svc;
    svc.set_time_scale(1.0 / 3.0);
    constexpr std::int64_t N = 60000; // 名义 20000 游戏秒 = 333 分钟
    push(svc, N);
    const std::int64_t expected = static_cast<std::int64_t>(std::floor(N / 3.0 / 60.0));
    EXPECT_NEAR(svc.session_total(), static_cast<double>(expected), 1.0); // 漂移 ≤1 分钟
}

TEST(Precision, MixedSpeedNoSystematicDrift) // TS-PR-004 中途变速漂移有界
{
    mud::TimeService svc;
    double acc = 0.0; // 独立累加的游戏秒
    const struct
    {
        double scale;
        std::int64_t frames;
    } segs[] = {{60.0, 5000}, {1.0, 1200}, {600.0, 900}, {30.5, 700}};
    for (const auto& s : segs)
    {
        svc.set_time_scale(s.scale);
        push(svc, s.frames);
        acc += s.scale * double(s.frames);
    }
    const std::int64_t expected = static_cast<std::int64_t>(std::floor(acc / 60.0));
    EXPECT_NEAR(svc.session_total(), static_cast<double>(expected), 1.0);
}

TEST(Precision, LongRunMonotonicAndExact) // TS-PR-007 长跑时间戳单调无回退
{
    mud::TimeService svc;
    svc.set_time_scale(60.0);
    std::int64_t last = -1;
    for (std::int64_t i = 0; i < 50000; ++i)
    {
        svc.update();
        const auto s = svc.session_total();
        ASSERT_GT(s, last);
        last = s;
    }
    EXPECT_EQ(last, 50000);
}