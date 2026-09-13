// 浇水与生长联动：浇水后生长推进、未浇水停滞。
#include <gtest/gtest.h>

#include "FarmLand.h"
#include "Cabbage.h"

// 未浇水不生长；浇水后按白天全速/夜间半速推进
TEST(FarmLandGrow, WateredLandGrowsAndDryLandStalls)
{
    FarmLand dry, wet;
    Cabbage seed1, seed2;
    ASSERT_TRUE(dry.sow(&seed1));
    ASSERT_TRUE(wet.sow(&seed2));

    dry.tickGrow(true);
    EXPECT_EQ(dry.getGrowthStage(), 0);      // 缺水停滞

    ASSERT_TRUE(wet.water());
    wet.tickGrow(false);                      // 夜间半速 +1
    EXPECT_EQ(wet.getGrowthStage(), 1);
    wet.tickGrow(false);                      // 夜间半速 +1
    EXPECT_EQ(wet.getGrowthStage(), 2);
}

// 生长阶段钳制到周期上限（远端合并的 DEF-008 钳制行为）
TEST(FarmLandGrow, GrowthStageClampedToCycle)
{
    FarmLand land;
    Cabbage seed;
    ASSERT_TRUE(land.sow(&seed));
    ASSERT_TRUE(land.water());

    land.tickGrow(true);                      // 白天全速 +2 → 达周期 2
    land.tickGrow(true);                      // 不再叠加，钳制在 2
    EXPECT_EQ(land.getGrowthStage(), 2);
    EXPECT_EQ(land.getGrowthStage(), land.getCrop()->getGrowthCycle());
}

// 生长完成后收获：产量正确、地块复位
TEST(FarmLandGrow, HarvestResetsLand)
{
    FarmLand land;
    Cabbage seed;
    ASSERT_TRUE(land.sow(&seed));
    ASSERT_TRUE(land.water());

    land.tickGrow(true);                      // stage=2 = 周期，可收获
    EXPECT_EQ(land.harvest(), 2);             // Cabbage 产量 2
    EXPECT_FALSE(land.isOccupied());
    EXPECT_FALSE(land.isWatered());
    EXPECT_EQ(land.getGrowthStage(), 0);
}

// 未达周期收获返回 0
TEST(FarmLandGrow, EarlyHarvestReturnsZero)
{
    FarmLand land;
    Cabbage seed;
    ASSERT_TRUE(land.sow(&seed));
    ASSERT_TRUE(land.water());

    land.tickGrow(false);                      // stage=1 < 周期 2
    EXPECT_EQ(land.harvest(), 0);
    EXPECT_TRUE(land.isOccupied());
}
