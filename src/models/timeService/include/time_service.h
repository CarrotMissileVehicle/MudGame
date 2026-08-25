#pragma once

#include "game_time.h"
#include "time_event.h"

#include <functional>

namespace mud {

    class TimeService {
    public:
        using Listener = std::function<void(TimeEvent)>;

    public:
        TimeService();

        gameTimePoint now() const;

        void tick(gameDuration real_delta);

        void set_time(gameTimePoint time);

        void set_time_scale(double scale);

        double time_scale() const;

        void subscribe(Listener listener);

    private:
        gameTimePoint current_time_;
        double time_scale_{1.0};

        // 具体实现暂时隐藏
    };

}