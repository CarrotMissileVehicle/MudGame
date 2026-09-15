/**
 * @file schedule_validation_test.cpp
 * @brief schedule_interval 周期参数校验测试：拒绝非正分钟数。
 */
#include "time_service.h"

#include <gtest/gtest.h>

#include <stdexcept>

TEST(ScheduleValidation, IntervalRejectsZeroMinutes)
{
    mud::TimeService svc;
    EXPECT_THROW(svc.schedule_interval(0, [] {}), std::invalid_argument);
}

TEST(ScheduleValidation, IntervalRejectsNegativeMinutes)
{
    mud::TimeService svc;
    EXPECT_THROW(svc.schedule_interval(-5, [] {}), std::invalid_argument);
}

TEST(ScheduleValidation, IntervalAcceptsPositiveMinutes)
{
    mud::TimeService svc;
    int fired = 0;
    const auto token = svc.schedule_interval(5, [&] { ++fired; });
    EXPECT_NE(token, 0u);
    for (int i = 0; i < 5; ++i) svc.update();
    EXPECT_EQ(fired, 1);
}
