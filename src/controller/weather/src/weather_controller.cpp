#include "weather_controller.h"

#include <chrono>
#include <random>

namespace
{
    // 返回 1~100 的随机整数，供当日事件判定用
    int roll_1_to_100()
    {
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(1, 100);
        return dist(gen);
    }
}

WeatherController::WeatherController(mud::Farm& farm)
    : farm_(farm), last_weather_day_(-1), last_event_day_(-1)
{
}

void WeatherController::update(int day, int hour)
{
    // 1) 跨天（凌晨 6 点算新的一天开始）：生成当天天气
    if (day != last_weather_day_ && hour >= 6) {
        last_weather_day_ = day;
        weather_.generate_daily();
        if (weather_.auto_water()) {
            farm_.auto_water();   // 雨天自动浇水
        }
    }

    // 2) 早上 8 点触发当天随机事件（同日只触发一次）
    if (day != last_event_day_ && hour == 8) {
        last_event_day_ = day;
        // 目前"虫害/旅行商人"条件由调用方后续完善前先用占位：
        //   day_of_week 固定为 1，neglect_water 固定 false
        const int   day_of_week   = 1;
        const bool  neglect_water = false;
        event_.generate_daily_events(day_of_week, neglect_water, roll_1_to_100());
    }
}

// ---- 天气查询转发 ----
mud::weather::WeatherType WeatherController::weather() const { return weather_.current(); }

std::string WeatherController::weather_name() const { return weather_.current_name(); }

bool WeatherController::can_fish() const { return weather_.can_fish(); }

bool WeatherController::can_go_outside() const { return weather_.can_go_outside(); }

bool WeatherController::auto_water() const { return weather_.auto_water(); }

double WeatherController::crop_loss_rate() const { return weather_.crop_loss_rate(); }

double WeatherController::mining_exp_bonus() const { return weather_.mining_exp_bonus(); }

double WeatherController::fishing_penalty() const { return weather_.fishing_penalty(); }

// ---- 事件查询转发 ----
bool WeatherController::has_event(mud::event::EventType type) const
{
    return event_.has_event(type);
}

bool WeatherController::is_traveler_active() const
{
    return event_.is_traveler_active();
}

// 返回今天触发的所有"每日 08:00 类"事件中文名
std::vector<std::string> WeatherController::today_event_names() const
{
    std::vector<std::string> names;
    for (std::size_t i = 0; i < mud::event::kDailyEventConfigCount; ++i) {
        const auto& cfg = mud::event::kDailyEventConfigs[i];
        if (event_.has_event(cfg.type)) {
            names.emplace_back(cfg.name);
        }
    }
    return names;
}

void WeatherController::roll_mining_event(bool& found_chest, bool& cave_in)
{
    event_.roll_mining_event(found_chest, cave_in, roll_1_to_100());
}