/**
 * @file player_state_test.cpp
 * @brief PlayerState 状态模型测试：仅接受 StateCode 构造、状态读写。
 */
#include <gtest/gtest.h>

#include <algorithm>
#include <type_traits>

#include "PlayerState.h"
#include "PlayerStateCode.h"

TEST(PlayerStateTest, IntCodeRejectedAtCompileTime)
{
    // DEF-375：移除 int 重载后，裸整数无法再注入非法状态码
    static_assert(!std::is_constructible_v<PlayerState, int>,
                  "PlayerState 不允许从裸 int 构造");
    static_assert(std::is_constructible_v<PlayerState, StateCode>,
                  "PlayerState 必须从 StateCode 构造");
}

TEST(PlayerStateTest, StateCodeConstructorSetsState)
{
    PlayerState s(StateCode::Repairing);
    EXPECT_EQ(s.GetState(), StateCode::Repairing);
    PlayerState idle(StateCode::Waiting);
    EXPECT_EQ(idle.GetState(), StateCode::Waiting);
}

TEST(PlayerStateTest, SetStateUpdatesState)
{
    PlayerState s(StateCode::Waiting);
    s.SetState(StateCode::Fishing);
    EXPECT_EQ(s.GetState(), StateCode::Fishing);
    s.SetState(StateCode::Mining);
    EXPECT_EQ(s.GetState(), StateCode::Mining);
}

TEST(PlayerStateTest, AbleStatesByPositionExist)
{
    PlayerState s(StateCode::Waiting);
    const auto* states = s.GetAbleStatesByPos(AtCoast);
    ASSERT_NE(states, nullptr);
    EXPECT_NE(std::find(states->begin(), states->end(), StateCode::Fishing), states->end());
}
