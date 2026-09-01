#include "time_service.h"

// 构造：初始化当前游戏时间
mud::TimeService::TimeService()
{
    // TODO: 实现
}

// 返回当前游戏时间点
mud::time::gameTimePoint mud::TimeService::now() const
{
    // TODO: 实现
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