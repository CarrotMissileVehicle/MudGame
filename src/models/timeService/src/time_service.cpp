/**
 * @file time_service.cpp
 * @brief 游戏时间服务实现：帧驱动推进与定时回调调度。
 */
#include "time_service.h"

#include <algorithm>
#include <chrono>

namespace
{
    using mud::time::gameDuration;

    // 游戏世界纪元：0 年 1 月 1 日 00:00
    constexpr std::chrono::sys_days kGameEpoch = std::chrono::year{0} / 1 / 1;

    // 将累计游戏时长转换为游戏日历字段。
    mud::TimeService::CalendarFields to_fields(gameDuration ms)
    {
        using namespace std::chrono;
        const auto whole_days = floor<days>(ms);
        const auto tod = hh_mm_ss<milliseconds>{ms - whole_days};
        const auto ymd = year_month_day{kGameEpoch + whole_days};
        return {
            static_cast<int>(ymd.year()),
            static_cast<unsigned>(ymd.month()),
            static_cast<unsigned>(ymd.day()), static_cast<int>(tod.hours().count()),
            static_cast<int>(tod.minutes().count())
        };
    }

    // 按倍率折算推进时长
    gameDuration scaled(gameDuration delta, double scale)
    {
        using rep = gameDuration::rep;
        return gameDuration{static_cast<rep>(delta.count() * scale)}; // NOLINT(*-narrowing-conversions)
    }
} // namespace

// 初始化世界纪元偏移与初始累计时长
mud::TimeService::TimeService(time::gameTimePoint origin,
                              time::gameDuration total_runtime)
    : startTime_(origin), accumulated_(total_runtime)
{
}

// 返回当前游戏时间点 连续
mud::time::gameTimePoint mud::TimeService::now() const
{
    return startTime_ + accumulated_;
}

// 返回当前累计游戏总长
mud::time::gameDuration mud::TimeService::session_total() const
{
    return accumulated_;
}

// 每帧推进：累加 delta×倍率，并触发落入 (上帧时刻, 当前时刻] 窗口内的到期回调。
void mud::TimeService::update(mud::time::gameDuration real_delta)
{
    const auto before = now();
    accumulated_ += scaled(real_delta, time_scale_);
    const auto after = now();

    // 收集窗口内到期的回调（携带原 due 用于排序；周期条目就地重排下一期）。
    struct Due
    {
        std::size_t token;
        time::gameTimePoint due;
        Callback cb;
    };
    std::vector<Due> due;
    for (auto it = schedule_.begin(); it != schedule_.end();)
    {
        if (it->due > before && it->due <= after)
        {
            due.push_back({it->token, it->due, it->callback});
            if (it->period.count() > 0)
            {
                it->due += it->period; // 周期复用：重排下一期
                ++it;
            }
            else
            {
                it = schedule_.erase(it); // 一次性：移除
            }
        }
        else
        {
            ++it;
        }
    }

    // 同一帧内跨多个时刻：按 due 升序（同刻按 token）顺序触发。
    std::ranges::sort(due,
                      [](const Due& a, const Due& b)
                      {
                          if (a.due != b.due) return a.due < b.due;
                          return a.token < b.token;
                      });
    for (const auto& d : due) d.cb();
}

// 直接设置当前游戏时间；不得早于世界创始时刻。
void mud::TimeService::set_time(mud::time::gameTimePoint time)
{
    const auto t = std::chrono::duration_cast<gameDuration>(time - startTime_);
    accumulated_ = std::max(t, gameDuration{0});
}

// 设置时间流速倍率
void mud::TimeService::set_time_scale(double scale) { time_scale_ = scale; }

// 返回时间流速倍率
double mud::TimeService::time_scale() const { return time_scale_; }

// 返回当前游戏日历字段
mud::TimeService::CalendarFields mud::TimeService::calendar_fields() const
{
    return to_fields(accumulated_);
}

// 注册一次性定时回调
std::size_t mud::TimeService::schedule_time(time::gameTimePoint due, Callback callback)
{
    const auto token = next_token_++;
    schedule_.push_back(Entry{token, due, gameDuration{0}, std::move(callback)});
    return token;
}

// 注册周期定时回调：首次到期 = 当前时刻 + period
std::size_t mud::TimeService::schedule_interval(time::gameDuration period, Callback callback)
{
    const auto token = next_token_++;
    schedule_.push_back(Entry{token, now() + period, period, std::move(callback)});
    return token;
}

// 退订指定令牌的定时回调
void mud::TimeService::cancel(std::size_t token)
{
    std::erase_if(schedule_,
                  [token](const Entry& e) { return e.token == token; });
}