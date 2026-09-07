#include "weather.h"

#include <cstddef>
#include <cstdlib>

namespace mud::weather
{
    namespace
    {
        // 天气概率配置（与 proj.md 一致）
        const WeatherConfig kConfigs[] = {
            { WeatherType::Sunny,   "晴天",   0.40, false, 0.00, 0.00, 0.00, true,  true  },
            { WeatherType::Rain,    "小雨",   0.25, true,  0.00, 0.00, 0.10, true,  true  },
            { WeatherType::Cloudy,  "阴天",   0.20, false, 0.00, 0.20, 0.00, true,  true  },
            { WeatherType::Storm,   "暴风雨", 0.10, false, 0.10, 0.00, 0.00, false, true  },
            { WeatherType::Typhoon, "台风",   0.05, false, 0.35, 0.00, 0.00, false, false },
        };
        const std::size_t kCount = sizeof(kConfigs) / sizeof(kConfigs[0]);
    }

    void Weather::generate_daily()
    {
        // 简单随机：生成 0~1 的均匀小数，落在哪段累计概率就选哪种天气
        double roll = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
        double acc = 0.0;
        for (std::size_t i = 0; i < kCount; ++i) {
            acc += kConfigs[i].probability;
            if (roll < acc) {
                current_ = kConfigs[i].type;
                return;
            }
        }
        // 兜底：浮点边界遗漏时落到晴天
        current_ = WeatherType::Sunny;
    }

    WeatherType Weather::current() const noexcept { return current_; }

    std::string Weather::current_name() const
    {
        for (std::size_t i = 0; i < kCount; ++i) {
            if (kConfigs[i].type == current_) return kConfigs[i].name;
        }
        return "晴天";
    }

    bool Weather::can_fish() const noexcept
    {
        return current() == WeatherType::Sunny ||
               current() == WeatherType::Rain ||
               current() == WeatherType::Cloudy;
    }

    bool Weather::can_go_outside() const noexcept
    {
        return current() != WeatherType::Typhoon;
    }

    bool Weather::auto_water() const noexcept
    {
        return kConfigs[static_cast<std::size_t>(current())].auto_water;
    }

    double Weather::crop_loss_rate() const noexcept
    {
        return kConfigs[static_cast<std::size_t>(current())].crop_loss;
    }

    double Weather::mining_exp_bonus() const noexcept
    {
        return kConfigs[static_cast<std::size_t>(current())].mining_exp_bonus;
    }

    double Weather::fishing_penalty() const noexcept
    {
        return kConfigs[static_cast<std::size_t>(current())].fishing_penalty;
    }
}