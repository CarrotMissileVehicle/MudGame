/**
 * @file tui_state_test.cpp
 * @brief TuiState 日志轮转计数测试。
 *
 * 覆盖 C3 修复依赖的不变量：push_log 累计追加计数单调递增、
 * 达到 maxLogs 上限后按条数裁剪并累计裁剪计数，保证 LogView
 * 能持续检测新日志并补偿滚动偏移。
 */
#include <gtest/gtest.h>

#include "TuiState.h"

using namespace mud::tui;

TEST(TuiStateLog, CountersTrackAppendAndEvict)
{
    TuiState state;
    for (int i = 0; i < 2500; ++i)
        state.push_log("line " + std::to_string(i));

    // 全部追加被计数，队列裁剪至上限，累计裁剪数 = 追加数 - 上限
    EXPECT_EQ(state.totalLogsAppended, 2500u);
    EXPECT_EQ(state.logs.size(), state.maxLogs);
    EXPECT_EQ(state.logsEvicted, 2500u - state.maxLogs);

    // 队首为裁剪后仍存留的最旧行：log[maxLogs] 保留，更早的已淘汰
    EXPECT_EQ(state.logs.front(), "line " + std::to_string(state.logsEvicted));
    EXPECT_EQ(state.logs.back(), "line 2499");
}

TEST(TuiStateLog, AppendBelowLimitDoesNotEvict)
{
    TuiState state;
    state.push_log("a");
    state.push_log("b");

    EXPECT_EQ(state.totalLogsAppended, 2u);
    EXPECT_EQ(state.logsEvicted, 0u);
    EXPECT_EQ(state.logs.size(), 2u);
    EXPECT_EQ(state.logs.front(), "a");
}

TEST(TuiStateLog, EvictedCountCumulativeAcrossBatches)
{
    TuiState state;
    state.push_log("x"); // 1 条，未裁剪
    EXPECT_EQ(state.logsEvicted, 0u);

    for (int i = 0; i < state.maxLogs + 5; ++i)
        state.push_log("y"); // 累计追加 1+maxLogs+5 条，超出上限 maxLogs+6
    EXPECT_EQ(state.logsEvicted, 6u);
    EXPECT_EQ(state.totalLogsAppended, 1u + state.maxLogs + 5);
}
