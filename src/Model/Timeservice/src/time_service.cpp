/**
 * @file time_service.cpp
 * @brief 游戏时间服务实现：帧驱动推进与定时回调调度。
 */
#include "time_service.h"

#include <algorithm>
#include <cmath>

// 构造：初始化当前游戏时间
mud::TimeService::TimeService(time::GameDateTime initial) : now_(initial)
{
}

// 返回当前游戏时间（显式日历，无秒）
mud::time::GameDateTime mud::TimeService::now() const
{
    return now_;
}

// 返回距纪元的总分钟数（供持久化）
std::int64_t mud::TimeService::session_total() const
{
    return now_.total_minutes();
}

// 每帧推进一帧（1 现实秒）：亚分钟累积满 60 秒进位为 1 游戏分钟，
// 并触发落入（推进前时刻, 推进后时刻] 窗口内的到期回调。
void mud::TimeService::update()
{
    sub_minute_ += time_scale_;
    const int whole = static_cast<int>(std::floor(sub_minute_ / 60.0));
    sub_minute_ -= whole * 60.0;
    if (whole <= 0) return;

    const auto before = now_;
    now_.advance(whole);
    const auto after = now_;

    // 收集窗口内到期的回调（周期条目就地重排下一期）。
    struct Due
    {
        std::size_t token;
        time::GameDateTime due;
        Callback cb;
    };
    std::vector<Due> due;
    for (auto it = schedule_.begin(); it != schedule_.end();)
    {
        if (it->due > before && it->due <= after)
        {
            due.push_back({it->token, it->due, it->callback});
            if (it->period > 0)
            {
                it->due.advance(it->period); // 周期复用：重排下一期
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
    std::sort(due.begin(), due.end(),
              [](const Due& a, const Due& b)
              {
                  if (a.due != b.due) return a.due < b.due;
                  return a.token < b.token;
              });
    for (const auto& d : due) d.cb();
}

// 直接跳进指定分钟数（世界推进用）：清空亚分钟余数，不触发定时回调
void mud::TimeService::advance(std::int64_t minutes)
{
    now_.advance(minutes);
    sub_minute_ = 0.0;
}

// 直接设置当前游戏时间，并清空亚分钟余数
void mud::TimeService::set_time(time::GameDateTime t)
{
    now_ = t;
    sub_minute_ = 0.0;
}

// 设置时间流速倍率
void mud::TimeService::set_time_scale(double scale) { time_scale_ = scale; }

// 返回时间流速倍率
double mud::TimeService::time_scale() const { return time_scale_; }

// 注册一次性定时回调
std::size_t mud::TimeService::schedule_time(time::GameDateTime due, Callback callback)
{
    const auto token = next_token_++;
    schedule_.push_back(Entry{token, due, 0, std::move(callback)});
    return token;
}

// 注册周期定时回调：首次到期 = 当前时刻 + minutes 分钟
std::size_t mud::TimeService::schedule_interval(std::int64_t minutes, Callback callback)
{
    const auto token = next_token_++;
    auto due = now_;
    due.advance(minutes);
    schedule_.push_back(Entry{token, due, minutes, std::move(callback)});
    return token;
}

// 退订指定令牌的定时回调
void mud::TimeService::cancel(std::size_t token)
{
    std::erase_if(schedule_,
                  [token](const Entry& e) { return e.token == token; });
}