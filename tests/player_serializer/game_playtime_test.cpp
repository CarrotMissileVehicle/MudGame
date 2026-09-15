/**
 * @file game_playtime_test.cpp
 * @brief Game 游玩时长测试（H2/H5）。
 *
 * 覆盖：
 *   - 空闲态 setTotalPlayTime→getTotalPlayTime 往返（秒粒度接口不变）；
 *   - 会话进行中 setTotalPlayTime 不重复叠加当前会话时间（H5）；
 *   - 会话进行中 getTotalPlayTime 单调不减、不低于基数（H2 steady_clock 语义）。
 */
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "Game.h"

using namespace std::chrono;

TEST(GamePlayTime, SetAndGetRoundtripWhenIdle)
{
    Game game;
    game.setTotalPlayTime(seconds{1234});
    EXPECT_EQ(game.getTotalPlayTime().count(), 1234);
}

TEST(GamePlayTime, SetWhileActiveDoesNotDoubleCount)
{
    Game game;
    game.startSession();
    // 让当前会话流逝至少 1 秒：旧实现会把传入值覆盖为基数后又叠加会话时长
    std::this_thread::sleep_for(milliseconds{1050});
    game.setTotalPlayTime(seconds{50});
    // 传入值应即时生效（基数扣除已流逝的 1 秒，总时长仍为 50）
    EXPECT_EQ(game.getTotalPlayTime().count(), 50);
    game.endSession();
    EXPECT_EQ(game.getTotalPlayTime().count(), 50);
}

TEST(GamePlayTime, ActiveElapsedIsMonotonicAndNonNegative)
{
    Game game;
    game.setTotalPlayTime(seconds{10});
    game.startSession();
    const auto t0 = game.getTotalPlayTime().count();
    EXPECT_GE(t0, 10);
    std::this_thread::sleep_for(milliseconds{30});
    const auto t1 = game.getTotalPlayTime().count();
    EXPECT_GE(t1, t0); // 单调时钟：时长只增不减
    game.endSession();
    EXPECT_GE(game.getTotalPlayTime().count(), 10);
}
