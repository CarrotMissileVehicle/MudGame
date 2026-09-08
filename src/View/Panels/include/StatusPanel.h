/**
 * @file StatusPanel.h
 * @brief 玩家状态面板：渲染玩家属性、三系经验与背包（吸收原 StatusView）。
 */
#pragma once

#include <string>

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class StatusPanel
    {
    public:
        explicit StatusPanel(Renderer& r) : r_(r) {}

        /// 渲染完整玩家状态面板。
        void render(const PlayerStatus& status) const;

    private:
        [[nodiscard]] static std::string GetPositionName(PositionCode code);
        [[nodiscard]] static std::string GetStateName(StateCode code);

        Renderer& r_;
    };
} // namespace mud::view