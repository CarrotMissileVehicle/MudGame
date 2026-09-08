/**
 * @file event_system_test.cpp
 * @brief 天气/事件系统冲突修复测试。
 *
 * 修复目标：暴风雨/台风/小雨等天气类"事件"已归入天气系统（mud::weather），
 * 事件系统只保留与天气无关的独立事件（旅行商人 / 虫害）。
 * 本测试验证 generate_daily_events 不再产生任何天气类事件。
 */
#include <gtest/gtest.h>

#include "event.h"
#include "weather.h"

TEST(EventSystemTest, GenerateIgnoresWeatherTypeEvents)
{
    mud::event::EventSystem es;
    // 任意日期：即便随机数落在昔日"小雨/暴风雨/台风"区间，也不应触发天气事件
    es.generate_daily_events(1, false); // 周一，未忽略浇水
    EXPECT_FALSE(es.has_event(mud::event::EventType::Traveler));
    EXPECT_FALSE(es.has_event(mud::event::EventType::Pest));
    // 事件系统无天气类事件可查询（枚举中已移除 Storm/Typhoon/Rain）
    EXPECT_EQ(mud::event::kDailyEventConfigCount, 2u);
}

TEST(EventSystemTest, FridaySpawnsTravelerOnly)
{
    mud::event::EventSystem es;
    es.generate_daily_events(5, false); // 周五
    EXPECT_TRUE(es.has_event(mud::event::EventType::Traveler));
    EXPECT_FALSE(es.has_event(mud::event::EventType::Pest));
}

TEST(EventSystemTest, NeglectWaterSpawnsPestOnly)
{
    mud::event::EventSystem es;
    es.generate_daily_events(1, true); // 连续 3 天未浇水
    EXPECT_TRUE(es.has_event(mud::event::EventType::Pest));
    EXPECT_FALSE(es.has_event(mud::event::EventType::Traveler)); // 周一无旅行商人
}

TEST(EventSystemTest, GenerateClearsPreviousDay)
{
    mud::event::EventSystem es;
    es.generate_daily_events(5, true); // 周五 + 虫害
    EXPECT_TRUE(es.has_event(mud::event::EventType::Traveler));
    EXPECT_TRUE(es.has_event(mud::event::EventType::Pest));
    es.generate_daily_events(1, false); // 次日无任何事件
    EXPECT_FALSE(es.has_event(mud::event::EventType::Traveler));
    EXPECT_FALSE(es.has_event(mud::event::EventType::Pest));
}

TEST(EventSystemTest, WeatherSystemOwnsWeatherType)
{
    // 天气系统仍负责"小雨"这一天气，不再由事件系统重复登记
    mud::weather::Weather w;
    EXPECT_TRUE(w.current_name() == "晴天" || w.current_name() == "小雨" ||
                w.current_name() == "阴天" || w.current_name() == "暴风雨" ||
                w.current_name() == "台风");
    // 事件系统配置中已无天气类名称
    for (std::size_t i = 0; i < mud::event::kDailyEventConfigCount; ++i) {
        const char* n = mud::event::kDailyEventConfigs[i].name;
        EXPECT_TRUE(n == std::string("旅行商人") || n == std::string("虫害"));
    }
}