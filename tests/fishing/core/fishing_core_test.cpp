/**
 * @file fishing_core_test.cpp
 * @brief 钓鱼控制器黑盒测试。
 *
 * 覆盖：空鱼池返回空、满/零成功率的确定性、tickFish 概率统计、
 * 日夜减半语义（tick + setCatchRate 两条路径）、rollFish 权重分布。
 * 统计用例容差取 ≥6σ，误报概率 < 1e-9。
 */
#include <gtest/gtest.h>

#include <vector>

#include "FishingController.h"
#include "Fish.h"
#include "game_time.h"

namespace
{

mud::time::GameDateTime at_hour(unsigned h)
{
    return {0, 1, 1, h, 0};
}

int count_catches(FishingController& ctrl, int trials)
{
    int caught = 0;
    for (int i = 0; i < trials; ++i)
        if (ctrl.tickFish() != nullptr) ++caught;
    return caught;
}

} // namespace

TEST(FishingCore, EmptyPoolYieldsNull)
{
    FishingController ctrl({});
    EXPECT_EQ(ctrl.poolSize(), 0u);
    EXPECT_EQ(ctrl.tickFish(), nullptr);
    EXPECT_EQ(ctrl.rollFish(), nullptr);
}

TEST(FishingCore, FullRateAlwaysCatches)
{
    Fish a(0.5f, 1, 10, 5, 10);
    FishingController ctrl({&a}, 1.0f);
    for (int i = 0; i < 100; ++i) EXPECT_NE(ctrl.tickFish(), nullptr);
}

TEST(FishingCore, ZeroRateNeverCatches)
{
    Fish a(0.5f, 1, 10, 5, 10);
    FishingController ctrl({&a}, 0.0f);
    for (int i = 0; i < 100; ++i) EXPECT_EQ(ctrl.tickFish(), nullptr);
}

TEST(FishingCore, NightHalvesCatchRate)
{
    Fish a(0.5f, 1, 10, 5, 10);
    FishingController ctrl({&a}, 1.0f);

    ctrl.tick(at_hour(22));            // 夜间：1.0 → 生效 0.5
    const int nightCaught = count_catches(ctrl, 400);
    EXPECT_GT(nightCaught, 140);       // 期望 200，±60 ≈ 6σ
    EXPECT_LT(nightCaught, 260);

    ctrl.tick(at_hour(12));            // 白天：恢复 1.0
    EXPECT_EQ(count_catches(ctrl, 100), 100);
}

TEST(FishingCore, SetCatchRateKeepsDayNightSemantics)
{
    Fish a(0.5f, 1, 10, 5, 10);
    FishingController ctrl({&a}, 0.0f);
    ctrl.tick(at_hour(3));             // 夜间
    ctrl.setCatchRate(1.0f);           // 夜间设置 → 生效 0.5
    const int caught = count_catches(ctrl, 400);
    EXPECT_GT(caught, 140);
    EXPECT_LT(caught, 260);
}

TEST(FishingCore, RollFishRespectsWeights)
{
    Fish heavy(0.9f, 1, 10, 5, 10);
    Fish light(0.1f, 2, 12, 8, 15);
    FishingController ctrl({&heavy, &light});

    int heavyHits = 0;
    constexpr int kTrials = 2000;
    for (int i = 0; i < kTrials; ++i)
        if (ctrl.rollFish() == &heavy) ++heavyHits;
    // 权重 0.9/0.1 → 期望 1800，±80 ≈ 6σ
    EXPECT_NEAR(heavyHits, 1800, 80);
}
