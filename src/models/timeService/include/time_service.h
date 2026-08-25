#pragma once

#include "game_time.h"
#include "time_event.h"

#include <functional>

namespace mud
{

    class TimeService
    {
    public:
        using Listener = std::function<void(time::TimeEvent)>;

    public:
        TimeService();

        time::gameTimePoint now() const;

        void tick(time::gameDuration real_delta);

        void set_time(time::gameTimePoint time);

        void set_time_scale(double scale);

        double time_scale() const;

        void subscribe(Listener listener);

    private:
        time::gameTimePoint current_time_;
        double time_scale_{1.0};

        // 具体实现暂时隐藏
    };

}