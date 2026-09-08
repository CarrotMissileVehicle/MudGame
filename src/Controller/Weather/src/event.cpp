#include "event.h"

#include <cstddef>
#include <cstdlib>

namespace mud::event
{
    void EventSystem::generate_daily_events(int day_of_week, bool neglect_water)
    {
        active_.clear();

        // 说明：暴风雨/台风/小雨已由天气系统（mud::weather）统一处理，
        // 不再在事件系统中重复触发生成，避免"同一天双轨报道同一场雨/风"。
        // 事件系统每日只保留与天气无关的独立事件。

        // 虫害：累计 3 天未浇水 → 独立于自然事件触发
        if (neglect_water) active_.insert(EventType::Pest);

        // 旅行商人：每周五 100% 在场 → 独立于自然事件触发
        if (day_of_week == 5) active_.insert(EventType::Traveler);
    }

    bool EventSystem::has_event(EventType type) const noexcept
    {
        return active_.count(type) > 0;
    }

    bool EventSystem::is_traveler_active() const noexcept
    {
        return has_event(EventType::Traveler);
    }

    void EventSystem::roll_mining_event(bool& found_chest, bool& cave_in, int rand_chance) const
    {
        // 矿洞塌方：5%（rand_chance < 5，即 1-4）
        cave_in = (rand_chance < 5);
        // 挖到宝箱：8%（rand_chance >= 93，即 93-100，共 8 个取值），与塌方独立判定
        found_chest = (rand_chance >= 93);
    }
}