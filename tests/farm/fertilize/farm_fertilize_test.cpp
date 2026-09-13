// DEF-008 回归：施肥语义——按「周期/speedUp」直接补贴生长进度。
// 修复前：growthStage += growthStage / speedUp，播种后（stage=0）施肥完全无效。
#include <gtest/gtest.h>

#include "FarmLand.h"
#include "Cabbage.h"

namespace
{
    // Cabbage：生长周期 2、产量 2
    Cabbage make_seed() { return Cabbage{}; }
}

// 播种后立即施肥有效（修复点：修复前 stage=0 时施肥无效）
TEST(FarmLandFertilize, EffectiveRightAfterSowing)
{
    FarmLand land;
    Cabbage seed = make_seed();
    ASSERT_TRUE(land.sow(&seed));

    land.fertilize(2);                       // 周期 2 / 2 = 1 的进度补贴
    EXPECT_GT(land.getGrowthStage(), 0);     // 修复前此处为 0 → 失败
}

// 施肥补贴量 = 周期 / speedUp（最小 1）
TEST(FarmLandFertilize, BonusEqualsCycleDividedBySpeedUp)
{
    FarmLand land;
    Cabbage seed = make_seed();
    ASSERT_TRUE(land.sow(&seed));

    const int before = land.getGrowthStage();
    land.fertilize(2);                       // 周期 2：补贴 max(1, 2/2)=1
    EXPECT_EQ(land.getGrowthStage() - before, 1);
}

// 补贴后生长阶段钳制到周期上限（不越界）
TEST(FarmLandFertilize, ClampedToCycleUpperBound)
{
    FarmLand land;
    Cabbage seed = make_seed();
    ASSERT_TRUE(land.sow(&seed));

    land.fertilize(1);                       // 补贴 2/1=2 → 已达周期上限
    EXPECT_EQ(land.getGrowthStage(), land.getCrop()->getGrowthCycle());

    land.fertilize(2);                       // 再施不越界
    EXPECT_EQ(land.getGrowthStage(), land.getCrop()->getGrowthCycle());
}

// 施肥后达到周期上限即可收获
TEST(FarmLandFertilize, FertilizedCropHarvestable)
{
    FarmLand land;
    Cabbage seed = make_seed();
    ASSERT_TRUE(land.sow(&seed));

    land.fertilize(1);                       // 周期 2 一步到位
    EXPECT_GT(land.harvest(), 0);
    EXPECT_FALSE(land.isOccupied());
}

// 空地施肥无效
TEST(FarmLandFertilize, NoEffectOnEmptyLand)
{
    FarmLand land;
    land.fertilize(2);
    EXPECT_FALSE(land.isOccupied());
    EXPECT_EQ(land.getGrowthStage(), 0);
}

// 非法 speedUp（<=0）安全忽略
TEST(FarmLandFertilize, InvalidSpeedUpIgnored)
{
    FarmLand land;
    Cabbage seed = make_seed();
    ASSERT_TRUE(land.sow(&seed));
    land.water();

    land.fertilize(0);
    land.fertilize(-3);
    EXPECT_EQ(land.getGrowthStage(), 0);
}
