#pragma once

#include <string>

namespace mud::weather
{
    enum class WeatherType
    {
        Sunny,     // 晴天：正常
        Rain,      // 小雨：农田自动浇水，钓鱼 -10%
        Cloudy,    // 阴天：采矿经验 +20%
        Storm,     // 暴风雨：农作物减产 10%，不能钓鱼
        Typhoon    // 台风：农作物减产 35%，不能外出
    };

    // 天气静态配置表（概率按 proj.md）
    struct WeatherConfig
    {
        WeatherType type;
        const char* name;
        double probability;        // 当日出现概率
        bool auto_water;           // 是否自动浇水
        double crop_loss;          // 农作物减产比例
        double mining_exp_bonus;   // 采矿经验加成
        double fishing_penalty;    // 钓鱼成功率减益
        bool can_fish;             // 能否钓鱼
        bool can_go_outside;       // 能否外出
    };

    // 天气 Model：纯数据 + 纯逻辑，不感知 UI / 输入
    class Weather
    {
    public:
        Weather() = default;

        // 依据概率随机生成当天天气
        void generate_daily();

        WeatherType current() const noexcept;
        std::string current_name() const;
        bool can_fish() const noexcept;
        bool can_go_outside() const noexcept;
        bool auto_water() const noexcept;
        double crop_loss_rate() const noexcept;
        double mining_exp_bonus() const noexcept;
        double fishing_penalty() const noexcept;

    private:
        WeatherType current_ = WeatherType::Sunny;
    };
}