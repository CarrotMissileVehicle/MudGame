#include "weather_controller.h"

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

// ---- 帧驱动入口：每帧调用 ----
// 参数是当前时刻的时间戳（绝对时间：hour 0-23、minute 0-59、day 为游戏天数），
// 不做增量累加，直接作为内部时钟，再据此驱动天气/事件逻辑。
// （游戏时间推进本身由调用方负责，本类只消费时间戳，不负责时间流逝。）
void WeatherController::frame_update(int day, int hour, int minute)
{
    current_.day    = day;
    current_.hour   = hour;
    current_.minute = minute;

    update(current_.day, current_.hour);
}

Time WeatherController::now() const
{
    return current_;
}

void WeatherController::update(int day, int hour)
{
    // 1) 跨天（凌晨 6 点算新的一天开始）：
    //    结算"上一天"是否浇水 → 更新连续未浇水天数；再生成当天天气，
    //    雨天自动浇水并视为当天已浇水。
    if (day != last_weather_day_ && hour >= 6) {
        // 上一天若无任何浇水（自动/手动都无）则计入未浇水天数，否则清零
        if (!watered_today_) {
            ++neglect_days_;
        } else {
            neglect_days_ = 0;
        }

        last_weather_day_ = day;
        weather_.generate_daily();
        watered_today_ = weather_.auto_water();   // 雨天自动浇水 = 当天已浇水
        if (weather_.auto_water()) {
            farm_.auto_water();   // 雨天自动浇水
        }
    }

    // 2) 早上 8 点触发当天随机事件（同日只触发一次）
    if (day != last_event_day_ && hour == 8) {
        last_event_day_ = day;
        const bool neglect_water = (neglect_days_ >= 3);   // 连续 3 天未浇水
        event_.generate_daily_events(day_of_week(day), neglect_water, roll_1_to_100());
    }
}

// 第 0 天为周一(1)，周末不对应；周五 = day%7==4 时返回 5。
int WeatherController::day_of_week(int day)
{
    return (day % 7) + 1;
}

void WeatherController::mark_watered()
{
    watered_today_ = true;
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