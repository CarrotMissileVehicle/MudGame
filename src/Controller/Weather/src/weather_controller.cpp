#include "weather_controller.h"

#include <cstddef>
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

WeatherController::WeatherController(mud::TimeService& time, Farm& farm)
    : time_(time), farm_(farm), last_weather_day_(-1), last_event_day_(-1)
{
}

// ---- 主循环心跳：每帧调用 ----
// 时间由 TimeService 唯一掌管（now_），本类只从 time_.now() 读当前时间驱动：
//   1) 跨天（凌晨 6 点算新一天开始）→ 结算上一天浇水情况、生成当天天气、雨天自动浇水
//   2) 早 8 点 → 触发当天随机事件（同日只触发一次）
void WeatherController::update()
{
    const auto t = time_.now();
    // 自纪元(0年1月1日)起的天序号：作为"今天是第几天/第几天"的稳定跨天判据（跨月/跨年都不会错判）
    const long long day_index = t.total_minutes() / 1440;
    const int hour = static_cast<int>(t.hour);

    // 1) 跨天：新的一天
    if (day_index != last_weather_day_ && hour >= 6) {
        // 上一天若无任何浇水（自动/手动都无）则计入未浇水天数，否则清零
        if (!watered_today_) {
            ++neglect_days_;
        } else {
            neglect_days_ = 0;
        }

        last_weather_day_ = day_index;
        weather_.generate_daily();
        watered_today_ = weather_.auto_water();   // 雨天自动浇水 = 当天已浇水
        if (weather_.auto_water()) {
            farm_.autoWater();   // 雨天给农田自动浇水
        }
    }

    // 2) 早 8 点触发当天随机事件（同日只触发一次）
    if (day_index != last_event_day_ && hour == 8) {
        last_event_day_ = day_index;
        const bool neglect_water = (neglect_days_ >= 3);   // 连续 3 天未浇水
        event_.generate_daily_events(day_of_week(day_index), neglect_water);
    }
}

// 由天序号常态化为星期几：第 0 天视为周一(1)，周五(5) 时旅行商人 100% 在场。
int WeatherController::day_of_week(long long day_index)
{
    return static_cast<int>((day_index % 7) + 1);
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