/**
 * @file MapPanel.h
 * @brief 地图面板：渲染当前位置。
 */
#pragma once

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class MapPanel
    {
    public:
        explicit MapPanel(Renderer& r) : r_(r) {}

        /// 渲染当前位置所在的地点。
        void render(PositionCode position) const;

    private:
        [[nodiscard]] static std::string PositionName(PositionCode code);

        Renderer& r_;
    };
} // namespace mud::view