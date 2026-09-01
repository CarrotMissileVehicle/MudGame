/**
 * @file time_service.cpp
 * @brief 游戏时间服务实现。
 */
#include "time_service.h"

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
    auto real_elapsed = std::chrono::steady_clock::now() - session_start_;
    auto real_ms =
        std::chrono::duration_cast<mud::time::gameDuration>(real_elapsed);
    auto scaled_ms = mud::time::gameDuration{
        static_cast<mud::time::gameDuration::rep>(real_ms.count() * time_scale_)};
    return startTime_ + total_runtime_ + scaled_ms;
}

// 返回含本次会话的当前累计总长
mud::time::gameDuration mud::TimeService::session_total() const
{
    auto real_elapsed = std::chrono::steady_clock::now() - session_start_;
    auto real_ms =
        std::chrono::duration_cast<mud::time::gameDuration>(real_elapsed);
    auto scaled_ms = mud::time::gameDuration{
        static_cast<mud::time::gameDuration::rep>(real_ms.count() * time_scale_)};
    return total_runtime_ + scaled_ms;
}

// 按帧驱动：统一比对两帧游戏时差，为时间事件判定提供刻度；不维护状态钟。
void mud::TimeService::tick(mud::time::gameDuration /*real_delta*/)
{
    // now() 已是实时推导且含倍率，tick 不再二次缩放，仅比对两帧游戏时差。
    const auto cur = now();
    const auto delta = cur - last_tick_time_; // 游戏时差
    last_tick_time_ = cur;
    // TODO: 依据 delta 判定跨日/整点等时间事件并通知订阅者
    (void)delta;
}

// 直接设置当前游戏时间
void mud::TimeService::set_time(mud::time::gameTimePoint time)
{
    
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

// 注册时间事件监听器
void mud::TimeService::subscribe(Listener listener)
{
    // TODO: 实现
}