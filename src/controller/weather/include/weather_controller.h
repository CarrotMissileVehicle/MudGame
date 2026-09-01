#pragma once

// ==========================================================================
// 天气与随机事件 Controller（mud 风格）
// 职责：把"时间 / 天气 / 农田"串起来：
//   - 跨天(到 6 点) → 生成当天天气；雨天自动给农田浇水
//   - 早上 8:00 → 触发当天随机事件
// 依赖：Farm 通过构造函数注入；时间通过 update(day, hour) 由调用方(GameController)
//       传入，避免依赖尚未实现的玩家/时间 Model。
// ==========================================================================

#include <string>
#include <vector>

#include "weather.h"   // mud::weather
#include "event.h"     // mud::event
#include "farm.h"      // mud::Farm（留桩）

class WeatherController
{
public:
    explicit WeatherController(mud::Farm& farm);

    // 每帧调用。跨天则生成当天天气，到早 8 点则触发当天事件。
    void update(int day, int hour);

    // ---- 天气查询 ----
    mud::weather::WeatherType weather() const;
    std::string weather_name() const;
    bool can_fish() const;
    bool can_go_outside() const;
    bool auto_water() const;
    double crop_loss_rate() const;
    double mining_exp_bonus() const;
    double fishing_penalty() const;

    // ---- 事件查询 ----
    bool has_event(mud::event::EventType type) const;
    bool is_traveler_active() const;
    std::vector<std::string> today_event_names() const;

    // 采矿随机事件（宝箱/塌方）
    void roll_mining_event(bool& found_chest, bool& cave_in);

private:
    mud::Farm& farm_;

    mud::weather::Weather weather_;
    mud::event::EventSystem event_;

    int last_weather_day_ = -1;   // 上次生成天气的天
    int last_event_day_ = -1;     // 上次触发事件的天
};