/**
 * @file weather_controller_test.cpp
 * @brief WeatherController 跨天推进测试（DEF-347）。
 *
 * 帧推进一次可跨多天（time_scale>1 / 睡觉 / 调试跳过）：
 * 中间天没有任何浇水动作，必须逐日补算未浇水天数，
 * 否则 neglect_days_ >= 3 的虫害判定因漏计而永不触发。
 */
#include <gtest/gtest.h>

#include "time_service.h"
#include "weather_controller.h"

TEST(WeatherControllerTest, MultiDayJumpAccumulatesNeglectDays)
{
    mud::TimeService time;
    Farm farm({});
    WeatherController wc(time, farm);

    // 帧推进直接跨到第 5 天 08:00：中间 5 天无任何浇水
    mud::time::GameDateTime t;
    t.advance(5 * 1440 + 8 * 60);
    time.set_time(t);

    wc.update();

    // 连续 ≥3 天未浇水 → 当日触发虫害事件
    EXPECT_TRUE(wc.has_event(mud::event::EventType::Pest));
}

TEST(WeatherControllerTest, SingleDayAdvanceNoFalsePest)
{
    mud::TimeService time;
    Farm farm({});
    WeatherController wc(time, farm);

    // 跨到第 2 天 08:00：仅跳过 1 天（上一天+中间 0 天）
    mud::time::GameDateTime t;
    t.advance(1 * 1440 + 8 * 60);
    time.set_time(t);

    wc.update();

    // 开局仅 1 天未浇水，不应触发虫害
    EXPECT_FALSE(wc.has_event(mud::event::EventType::Pest));
}
