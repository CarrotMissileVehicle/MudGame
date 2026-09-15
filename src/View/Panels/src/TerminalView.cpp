/**
 * @file TerminalView.cpp
 * @brief 终端视图门面实现。
 */
#include "TerminalView.h"

#include "view_primitives.h"

namespace mud::view
{
    void TerminalView::render_now(const TimeView& t, const WeatherView& w) const
    {
        time_.render(t);
        r_.print("今日天气：" + w.weather);
    }

    void TerminalView::render_all(const GameSnapshot& snap) const
    {
        if (!snap.messages.empty())
        {
            messagePanel_.render(snap.messages);
            r_.print("");
        }

        // 天气由下方 weather_.render() 统一输出，避免与 render_now 的"今日天气"重复
        time_.render(snap.time);
        status_.render(snap.player);

        if (!snap.farm.plots.empty())
        {
            farm_.render(snap.farm);
            r_.print("");
        }

        if (!snap.market.shops.empty())
        {
            market_.render(snap.market);
            r_.print("");
        }

        if (!snap.tools.tools.empty())
            tools_.render(snap.tools);

        mining_.render_status(snap.mining);
        fishing_.render(snap.fishing);
        weather_.render(snap.weather);
    }
} // namespace mud::view