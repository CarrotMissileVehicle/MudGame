/**
 * @file event.cpp
 * @brief 事件模型实现（mud::event）。
 */
#include "event.h"

namespace mud::event
{
    void EventSystem::roll_mining_event(
        bool& found_chest,
        bool& cave_in,
        int rand_chance
    ) const
    {
        cave_in      = (rand_chance < 5);
        found_chest  = (rand_chance >= 93);
    }
}