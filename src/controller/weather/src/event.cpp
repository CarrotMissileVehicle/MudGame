#include "event.h"

#include <cstddef>
#include <cstdlib>

namespace mud::event
{
    namespace
    {
        // 事件概率配置（proj.md）：以 1~100 的随机整数判定
        struct DailyProb
        {
            EventType type;
            int chance_percent;   // 触发概率（0-100）
        };

        const DailyProb kDailyProb[] = {
            { EventType::Storm,   8 },
            { EventType::Typhoon, 3 },
            { EventType::Rain,    20 },
        };
        const std::size_t kDailyProbCount = sizeof(kDailyProb) / sizeof(kDailyProb[0]);
    }

    void EventSystem::generate_daily_events(int day_of_week, bool neglect_water, int rand_chance)
    {
        active_.clear();

        // 概率类事件采用"加权轮盘"：把暴风雨/台风/小雨按概率累积成一段段区间，
        // rand_chance(1-100) 落在哪段就触发哪一类——每天 08:00 至多触发一个自然事件，
        // 避免多个事件共用同一随机数造成互斥与概率失真。
        // 累积区间：暴风雨 8% (1-8)、台风 3% (9-11)、小雨 20% (12-31)
        int acc = 0;
        for (std::size_t i = 0; i < kDailyProbCount; ++i) {
            acc += kDailyProb[i].chance_percent;
            if (rand_chance > 0 && rand_chance <= acc) {
                active_.insert(kDailyProb[i].type);
                break;
            }
        }

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

    void EventSystem::roll_mining_event(bool& found_chest, bool& cave_in, int rand_chance)
    {
        // 矿洞塌方：5%
        cave_in = (rand_chance < 5);
        // 挖到宝箱：采矿时随机（简单固定 8%，与塌方独立判定）
        found_chest = (rand_chance >= 80);
    }
}