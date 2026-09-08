/**
 * @file WeatherPanel.h
 * @brief 天气面板：渲染今日天气、影响数值与当日事件（收编 main 的 weather.now）。
 */
#pragma once

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class WeatherPanel
    {
    public:
        explicit WeatherPanel(Renderer& r) : r_(r) {}

        /// 渲染天气与事件详情（今日天气由调用方与时间面板组合输出）。
        void render(const WeatherView& weather) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view