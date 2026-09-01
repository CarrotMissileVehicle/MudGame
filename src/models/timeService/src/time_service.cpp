/**
 * @file time_service.cpp
 * @brief 游戏时间服务实现。
 */
#include "time_service.h"

// 构造：初始化当前游戏时间
mud::TimeService::TimeService(mud::time::gameTimePoint origin,
                              mud::time::gameDuration total_runtime,
                              mud::time::gameTimePoint session_start)
    : startTime_(origin), session_start_(session_start), total_runtime_(total_runtime)
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

// 返回含本次会话的当前累计总长（供调用方持久化）
mud::time::gameDuration mud::TimeService::session_total() const
{
    auto real_elapsed = std::chrono::steady_clock::now() - session_start_;
    auto real_ms =
        std::chrono::duration_cast<mud::time::gameDuration>(real_elapsed);
    auto scaled_ms = mud::time::gameDuration{
        static_cast<mud::time::gameDuration::rep>(real_ms.count() * time_scale_)};
    return total_runtime_ + scaled_ms;
}

// 按真实经过时长推进游戏时间（受时间倍率影响）
void mud::TimeService::tick(mud::time::gameDuration real_delta)
{
    // TODO: 实现
}

// 直接设置当前游戏时间
void mud::TimeService::set_time(mud::time::gameTimePoint time)
{
    // TODO: 实现
}

// 设置游戏时间流速倍率
void mud::TimeService::set_time_scale(double scale)
{
    // TODO: 实现
}

// 返回当前时间流速倍率
double mud::TimeService::time_scale() const
{
    // TODO: 实现
}

// 注册时间事件监听器
void mud::TimeService::subscribe(Listener listener)
{
    // TODO: 实现
}