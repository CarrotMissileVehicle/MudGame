/**
 * @file position_test.cpp
 * @brief 地图导航测试。
 *
 * H13：基类 Position::Go* 此前静默返回 nullptr，任何
 * "pos->GoUp()->GetCode()" 式调用都会解引用崩溃；改为显式抛
 * std::logic_error。具体位置（Home/Town/...）按移动图覆盖各自方向。
 */
#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include "Position.h"
#include "Home.h"
#include "Town.h"
#include "Coast.h"
#include "Mine.h"

TEST(PositionNavTest, BaseNavigationThrowsInsteadOfNullptr)
{
    Position base(PositionCode::AtHome);
    EXPECT_THROW(base.GoUp(), std::logic_error);
    EXPECT_THROW(base.GoDown(), std::logic_error);
    EXPECT_THROW(base.GoLeft(), std::logic_error);
    EXPECT_THROW(base.GoRight(), std::logic_error);
}

TEST(PositionNavTest, HomeOverridesAllDirections)
{
    Home home;
    {
        std::unique_ptr<Position> up(home.GoUp());
        ASSERT_NE(up, nullptr);
        EXPECT_EQ(up->GetCode(), PositionCode::AtTown);
    }
    {
        std::unique_ptr<Position> down(home.GoDown());
        ASSERT_NE(down, nullptr);
        EXPECT_EQ(down->GetCode(), PositionCode::AtMine);
    }
    {
        std::unique_ptr<Position> left(home.GoLeft());
        ASSERT_NE(left, nullptr);
        EXPECT_EQ(left->GetCode(), PositionCode::AtFarmland);
    }
    {
        std::unique_ptr<Position> right(home.GoRight());
        ASSERT_NE(right, nullptr);
        EXPECT_EQ(right->GetCode(), PositionCode::AtCoast);
    }
}

TEST(PositionNavTest, UncoveredDirectionThrows)
{
    // Coast 仅覆盖 GoLeft（回 Home）；其余方向走基类 → 显式抛错而非空指针
    Coast coast;
    EXPECT_THROW(coast.GoUp(), std::logic_error);
    EXPECT_THROW(coast.GoDown(), std::logic_error);
    EXPECT_THROW(coast.GoRight(), std::logic_error);
    {
        std::unique_ptr<Position> left(coast.GoLeft());
        ASSERT_NE(left, nullptr);
        EXPECT_EQ(left->GetCode(), PositionCode::AtHome);
    }

    // Mine 仅覆盖 GoUp（回 Home）
    Mine mine;
    EXPECT_THROW(mine.GoDown(), std::logic_error);
    EXPECT_THROW(mine.GoLeft(), std::logic_error);
    EXPECT_THROW(mine.GoRight(), std::logic_error);
    {
        std::unique_ptr<Position> up(mine.GoUp());
        ASSERT_NE(up, nullptr);
        EXPECT_EQ(up->GetCode(), PositionCode::AtHome);
    }
}
