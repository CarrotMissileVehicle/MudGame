/**
 * @file player_state_test.cpp
 * @brief PlayerState 状态模型测试：int 构造参数不再被忽略、状态读写。
 */
#include <gtest/gtest.h>

#include <algorithm>

#include "PlayerState.h"
#include "PlayerStateCode.h"

TEST(PlayerStateTest, IntConstructorUsesCodeParam)
{
    PlayerState s(static_cast<int>(Moving));
    EXPECT_EQ(s.GetState(), Moving);
    PlayerState s2(static_cast<int>(Sleeping));
    EXPECT_EQ(s2.GetState(), Sleeping);
    PlayerState s3(static_cast<int>(Mining));
    EXPECT_EQ(s3.GetState(), Mining);
}

TEST(PlayerStateTest, StateCodeConstructorSetsState)
{
    PlayerState s(Repairing);
    EXPECT_EQ(s.GetState(), Repairing);
    PlayerState idle(Waiting);
    EXPECT_EQ(idle.GetState(), Waiting);
}

TEST(PlayerStateTest, SetStateUpdatesState)
{
    PlayerState s(Waiting);
    s.SetState(Fishing);
    EXPECT_EQ(s.GetState(), Fishing);
    s.SetState(Mining);
    EXPECT_EQ(s.GetState(), Mining);
}

TEST(PlayerStateTest, AbleStatesByPositionExist)
{
    PlayerState s(Waiting);
    const auto* states = s.GetAbleStatesByPos(AtCoast);
    ASSERT_NE(states, nullptr);
    EXPECT_NE(std::find(states->begin(), states->end(), Fishing), states->end());
}
