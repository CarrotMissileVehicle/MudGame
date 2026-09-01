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

#include "model/Time/include/Time.h"   // 游戏时钟（天/时/分）；用路径包含避免与系统 <time.h> 冲突
#include "weather.h"   // mud::weather
#include "event.h"     // mud::event
#include "farm.h"      // mud::Farm（留桩）

class WeatherController
{
public:
    explicit WeatherController(mud::Farm& farm);

    // ---- 帧驱动入口：每帧调用 ----
    // 参数是当前时刻的时间戳（绝对时间：day=游戏天数、hour 0-23、minute 0-59），
    // 不做增量累加。游戏时间推进由调用方(GameController)负责，本类仅消费时间戳，
    // 并据此驱动天气与随机事件：跨天(凌晨6点)生成天气、到早 8 点触发当天事件。
    void frame_update(int day, int hour, int minute);

    // 直接以"游戏天数/小时"驱动（等同 frame_update 的推进后的瞬时，供非帧驱动场景复用）。
    void update(int day, int hour);

    // 当前游戏时钟（day=游戏第几天，hour 0-23，minute 0-59）
    Time now() const;

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

    // 手动浇水标记：由种菜系统在玩家主动浇水(非雨天)时调用，
    // 用于重置"连续未浇水天数"计数（否则会错误累计成虫害）。
    void mark_watered();

    // 采矿随机事件（宝箱/塌方）
    void roll_mining_event(bool& found_chest, bool& cave_in);

private:
    // 从游戏天数判定星期几（第 0 天=周一，周五=5），供旅行商人触发。
    static int day_of_week(int day);

private:
    mud::Farm& farm_;

    mud::weather::Weather weather_;
    mud::event::EventSystem event_;

    int last_weather_day_ = -1;   // 上次生成天气的天
    int last_event_day_ = -1;     // 上次触发事件的天
    int neglect_days_ = 0;        // 连续未浇水天数（≥3 触发虫害）
    bool watered_today_ = false;  // 当天是否已浇水（雨天自动浇水或玩家手动）
    Time current_{ 0, 0, 0 };     // 内部游戏时钟，由 frame_update 累加推进
};