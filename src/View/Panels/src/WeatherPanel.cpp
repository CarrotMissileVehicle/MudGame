/**
 * @file WeatherPanel.cpp
 * @brief 天气面板实现。
 */
#include "WeatherPanel.h"

#include <sstream>

namespace mud::view
{
    namespace
    {
        /// 以流默认精度输出 double（避免 std::to_string 尾零过长）。
        std::string fmt_double(double v)
        {
            std::ostringstream os;
            os << v;
            return os.str();
        }
    } // namespace

    void WeatherPanel::render(const WeatherView& weather) const
    {
        r_.print("今日天气：" + weather.weather);
        r_.print("自动浇水：" + std::string(weather.auto_water ? "是" : "否"));
        r_.print("作物减产率：" + fmt_double(weather.crop_loss_rate));
        r_.print("采矿经验加成：" + fmt_double(weather.mining_exp_bonus));
        r_.print("垂钓减益：" + fmt_double(weather.fishing_penalty));
        if (!weather.events.empty())
        {
            std::string line = "今日事件：";
            for (const auto& e : weather.events)
            {
                line += e;
                line += " ";
            }
            r_.print(line);
        }
    }
} // namespace mud::view