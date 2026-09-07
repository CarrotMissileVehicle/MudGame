/**
 * @file weather_controller.h
 * @brief 天气与随机事件控制器（WeatherController）。
 *
 * 职责：把"时间 / 天气 / 农田"串起来：
 *   - 跨天（到凌晨 6 点）→ 生成当天天气；雨天自动给农田浇水
 *   - 早上 8:00 → 触发当天随机事件
 *
 * 依赖通过构造函数注入：时间来自 master 的 mud::TimeService（update() 内读其 now() 驱动），
 * 农田来自 master 的全局 Farm（雨天自动浇水）。
 * 依赖方向：Controller → Model，Model 不感知上层。
 */
#pragma once

#include <string>
#include <vector>

#include "time_service.h"  // mud::TimeService（由 time_service 目标提供 include）
#include "weather.h"   // mud::weather
#include "event.h"     // mud::event
#include "farm.h"      // 全局 Farm（master 农田骨架）

class WeatherController
{
public:
    // 依赖注入：时间服务 + 农田
    WeatherController(mud::TimeService& time, Farm& farm);

    // 主循环每帧调用：从 time_.now() 读取当前游戏时间并驱动天气/事件。
    //   - 跨天（到新的一天 6 点）→ 生成当天天气，雨天自动浇水
    //   - 到早 8 点 → 触发当天随机事件
    void update();

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

    // 采矿时的随机事件判定（宝箱 / 塌方）
    void roll_mining_event(bool& found_chest, bool& cave_in);

private:
    // 由"自纪元起始的天数"常态化为 1-7（第 0 天视为周一，周五=5），供旅行商人触发
    static int day_of_week(long long day_index);

    mud::TimeService& time_;   // 注入的时间服务（引用）
    Farm& farm_;               // 注入的农田（引用）

    mud::weather::Weather weather_;
    mud::event::EventSystem event_;

    long long last_weather_day_ = -1;   // 上次生成天气的自纪元天数
    long long last_event_day_   = -1;   // 上次触发事件的自纪元天数
    int neglect_days_ = 0;              // 连续未浇水天数（≥3 触发虫害）
    bool watered_today_ = false;        // 当天是否已浇水（雨天自动浇水或玩家手动）
};