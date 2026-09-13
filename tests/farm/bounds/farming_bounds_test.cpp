/**
 * @file farming_bounds_test.cpp
 * @brief 农场控制器索引边界测试（DEF-108）。
 *
 * 命令层传入的地块索引来自用户输入：越界时 FarmingController 必须
 * 以返回值拒绝（false / 0），而非令 Farm::getFarmland().at() 抛出
 * std::out_of_range 未捕获异常导致游戏崩溃。
 */
#include <gtest/gtest.h>

#include <vector>

#include "FarmingController.h"
#include "FarmLand.h"
#include "Cabbage.h"
#include "NormalFertilizer.h"

namespace
{

struct TwoLandsFarm
{
    std::vector<FarmLand> lands{FarmLand{}, FarmLand{}};
    Farm farm{lands};
    FarmingController controller{&farm};
};

} // namespace

TEST(FarmingBounds, SowOutOfBoundsRejectedNoThrow) // DEF-108
{
    TwoLandsFarm f;
    Cabbage seed;
    EXPECT_FALSE(f.controller.sow(2, &seed));    // 越界（size=2）
    EXPECT_FALSE(f.controller.sow(9999, &seed));  // 远越界
}

TEST(FarmingBounds, WaterOutOfBoundsRejectedNoThrow) // DEF-108
{
    TwoLandsFarm f;
    EXPECT_FALSE(f.controller.water(2));
    EXPECT_FALSE(f.controller.water(static_cast<std::size_t>(-1)));
}

TEST(FarmingBounds, FertilizeOutOfBoundsRejectedNoThrow) // DEF-108
{
    TwoLandsFarm f;
    NormalFertilizer fert;
    EXPECT_FALSE(f.controller.fertilize(2, &fert));
}

TEST(FarmingBounds, HarvestOutOfBoundsReturnsZeroNoThrow) // DEF-108
{
    TwoLandsFarm f;
    EXPECT_EQ(f.controller.harvest(2), 0);
}

TEST(FarmingBounds, InBoundsOperationsStillWork) // 越界守卫不误伤合法路径
{
    TwoLandsFarm f;
    Cabbage seed;
    EXPECT_TRUE(f.controller.sow(0, &seed));   // 合法索引播种成功
    EXPECT_TRUE(f.controller.water(0));
    EXPECT_EQ(f.controller.farmSize(), 2u);
    NormalFertilizer fert;
    EXPECT_TRUE(f.controller.fertilize(0, &fert));
    // 空地收获返回 0（合法，非越界）
    EXPECT_EQ(f.controller.harvest(1), 0);
}
