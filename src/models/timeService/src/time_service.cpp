/**
 * @file time_service.cpp
 * @brief 游戏时间服务实现。
 */
#include "time_service.h"

#include <algorithm>
#include <chrono>

namespace
{
    using mud::time::gameDuration;

    // 游戏世界纪元：0 年 1 月 1 日 00:00:00（专有格里高利历，跨月/闰年与现实一致，无闰秒）。
    constexpr std::chrono::sys_days kGameEpoch = std::chrono::year{0} / 1 / 1;

    // 将累计游戏时长拆分为游戏日历字段，用于判定分/时/天/月/年过界。
    struct GameClockFields
    {
        int year;
        unsigned month;
        unsigned day;
        int hour;
        int minute;
    };

    GameClockFields game_clock_fields(gameDuration ms)
    {
        using namespace std::chrono;
        auto whole_days = floor<days>(ms);
        auto tod = hh_mm_ss<milliseconds>{ms - whole_days};
        auto ymd = year_month_day{kGameEpoch + whole_days};
        return {static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()),
                static_cast<unsigned>(ymd.day()), static_cast<int>(tod.hours().count()),
                static_cast<int>(tod.minutes().count())};
    }
} // namespace

// 构造：初始化当前游戏时间
mud::TimeService::TimeService(mud::time::gameTimePoint origin,
                              mud::time::gameDuration total_runtime,
                              mud::time::gameTimePoint session_start)
    : startTime_(origin), session_start_(session_start), last_tick_time_(session_start),
      total_runtime_(total_runtime)
{
}

// 返回当前游戏时间点
mud::time::gameTimePoint mud::TimeService::now() const
{
    return startTime_ + total_runtime_ + live_elapsed_scaled();
}

// 返回含本次会话的当前累计总长
mud::time::gameDuration mud::TimeService::session_total() const
{
    return total_runtime_ + live_elapsed_scaled();
}

// 按倍率折算的推进时长
mud::time::gameDuration mud::TimeService::live_elapsed_scaled() const
{
    auto real_elapsed = std::chrono::steady_clock::now() - session_start_;
    auto real_ms = std::chrono::duration_cast<mud::time::gameDuration>(real_elapsed);
    return mud::time::gameDuration{
        static_cast<mud::time::gameDuration::rep>(real_ms.count() * time_scale_)};
}

// 比对相邻两帧游戏日历，向订阅者派发过界时间事件
void mud::TimeService::tick(mud::time::gameDuration /*real_delta*/)
{
    const auto cur = now();
    const auto prev_ms = last_tick_time_ - startTime_;
    const auto cur_ms = cur - startTime_;
    last_tick_time_ = cur;

    if (listeners_.empty()) return;

    const auto before = game_clock_fields(
        std::chrono::duration_cast<mud::time::gameDuration>(prev_ms));
    const auto after = game_clock_fields(
        std::chrono::duration_cast<mud::time::gameDuration>(cur_ms));

    const auto notify = [this](time::TimeEvent ev)
    {
        for (const auto& entry : listeners_) entry.callback(ev);
    };
    // 按 分→时→日→月→年 升序派发，跨多粒度时各监听器逐级收到通知。
    if (after.minute != before.minute) notify(time::TimeEvent::MinuteChanged);
    if (after.hour != before.hour) notify(time::TimeEvent::HourChanged);
    if (after.day != before.day) notify(time::TimeEvent::DayChanged);
    if (after.month != before.month) notify(time::TimeEvent::MonthChanged);
    if (after.year != before.year) notify(time::TimeEvent::YearChanged);
}

// 直接设置当前游戏时间：反解累计总长使 now() 恰好等于 time；
void mud::TimeService::set_time(mud::time::gameTimePoint time)
{
    total_runtime_ = std::chrono::duration_cast<mud::time::gameDuration>(time - startTime_)
                     - live_elapsed_scaled();
    // 游戏时间不得早于世界创始时刻，钳制下限为 0。
    total_runtime_ = std::max(total_runtime_, mud::time::gameDuration{0});
    last_tick_time_ = now();
}

// 设置游戏时间流速倍率
void mud::TimeService::set_time_scale(double scale)
{
    time_scale_ = scale;
}

// 返回当前时间流速倍率
double mud::TimeService::time_scale() const
{
    return time_scale_;
}

// 注册时间事件监听器，返回退订令牌
std::size_t mud::TimeService::subscribe(Listener listener)
{
    const auto token = next_token_++;
    listeners_.push_back(Entry{token, std::move(listener)});
    return token;
}

// 退订指定令牌对应的时间事件监听器
void mud::TimeService::unsubscribe(std::size_t token)
{
    listeners_.erase(std::remove_if(listeners_.begin(), listeners_.end(),
                                    [token](const Entry& e) { return e.token == token; }),
                     listeners_.end());
}