/**
 * @file TerminalView.h
 * @brief 终端视图门面：持有全部面板，向 Controller/组合根暴露统一渲染入口。
 *
 * Controller → TerminalView（调 render，传 DTO）；
 * TerminalView 只依赖 Renderer 与 DTO，不触碰任何 Model/Controller。
 */
#pragma once

#include <string>
#include <vector>

#include "Renderer.h"
#include "dto.h"
#include "TimePanel.h"
#include "StatusPanel.h"
#include "MapPanel.h"
#include "FarmPanel.h"
#include "FishingPanel.h"
#include "MiningPanel.h"
#include "MarketPanel.h"
#include "ToolsPanel.h"
#include "BlacksmithPanel.h"
#include "WeatherPanel.h"
#include "HelpScreen.h"
#include "MessagePanel.h"
#include "view_primitives.h"

namespace mud::view
{
    class TerminalView
    {
    public:
        explicit TerminalView(Renderer& r)
            : r_(r), time_(r), status_(r), map_(r), farm_(r), fishing_(r),
              mining_(r), market_(r), tools_(r), blacksmith_(r), weather_(r), help_(r), messagePanel_(r)
        {
        }

        // ---- 面板门面 ----
        void render_time(const TimeView& t) const { time_.render(t); }
        void render_time_scale(double factor) const { time_.render_scale(factor); }
        void render_status(const PlayerStatus& s) const { status_.render(s); }
        void render_map(PositionCode p) const { map_.render(p); }
        void render_farm(const FarmView& f) const { farm_.render(f); }
        void render_fishing(const FishingView& f) const { fishing_.render(f); }
        void render_mining_status(const MiningView& m) const { mining_.render_status(m); }
        void render_mining_produce(const std::vector<MiningEventView>& r) const { mining_.render_produce(r); }
        void render_market(const MarketView& m) const { market_.render(m); }
        void render_tools(const ToolsView& t) const { tools_.render(t); }
        void render_blacksmith(const BlacksmithView& b) const { blacksmith_.render(b); }
        void render_weather(const WeatherView& w) const { weather_.render(w); }
        void render_help(const std::string& h) const { help_.render(h); }
        void render_message(const MessageLine& m) const { messagePanel_.render(m); }

        /// 组合渲染：时间 + 天气（对应原 print_now）。
        void render_now(const TimeView& t, const WeatherView& w) const;

        /// 一次性全量渲染主界面（启动/整帧刷新）。
        void render_all(const GameSnapshot& snap) const;

        /// 输出命令提示符。
        void print_prompt() const { mud::view::print_prompt(r_); }

    private:
        Renderer& r_;
        TimePanel time_;
        StatusPanel status_;
        MapPanel map_;
        FarmPanel farm_;
        FishingPanel fishing_;
        MiningPanel mining_;
        MarketPanel market_;
        ToolsPanel tools_;
        BlacksmithPanel blacksmith_;
        WeatherPanel weather_;
        HelpScreen help_;
        MessagePanel messagePanel_;
    };
} // namespace mud::view