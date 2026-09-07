#pragma once

#include <cstddef>
#include <string>
#include <unordered_set>

namespace mud::event
{
    enum class EventType
    {
        Storm,      // 暴风雨：农作物减产 10%
        Typhoon,    // 台风：农作物减产 35%
        Rain,       // 小雨：不用浇水
        Traveler,   // 旅行商人：出售稀有种子/道具
        Pest,       // 虫害：作物减产 20%
        Chest,      // 挖到宝箱：随机金币/矿石
        CaveIn      // 矿洞塌方：失去当日矿石
    };

    // 事件的触发范围：每日固定 08:00 触发 / 采矿过程中的随机事件
    enum class EventScope
    {
        Daily08,
        Mining
    };

    struct EventConfig
    {
        EventType type;
        EventScope scope;
        const char* name;
    };

    // 早 8:00 每日事件配置表
    inline constexpr EventConfig kDailyEventConfigs[] = {
        { EventType::Storm,    EventScope::Daily08, "暴风雨" },
        { EventType::Typhoon,  EventScope::Daily08, "台风"   },
        { EventType::Rain,     EventScope::Daily08, "小雨"   },
        { EventType::Traveler, EventScope::Daily08, "旅行商人" },
        { EventType::Pest,     EventScope::Daily08, "虫害"   },
    };
    inline constexpr std::size_t kDailyEventConfigCount =
        sizeof(kDailyEventConfigs) / sizeof(kDailyEventConfigs[0]);

    // 事件 Model：纯数据 + 纯逻辑
    class EventSystem
    {
    public:
        EventSystem() = default;

        // 每天 08:00 依据概率与条件生成当日触发的事件（事件判定规则按 proj.md）
        void generate_daily_events(int day_of_week, bool neglect_water, int rand_chance);

        bool has_event(EventType type) const noexcept;
        bool is_traveler_active() const noexcept;

        // 采矿时的随机事件判定（宝箱 / 塌方）
        void roll_mining_event(bool& found_chest, bool& cave_in, int rand_chance);

    private:
        std::unordered_set<EventType> active_;
    };
}