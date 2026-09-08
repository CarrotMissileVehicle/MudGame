/**
 * @file MiningPanel.h
 * @brief 采矿面板：渲染采矿状态与产出入库反馈（收编 main 的 mine.* 输出）。
 */
#pragma once

#include <vector>

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class MiningPanel
    {
    public:
        explicit MiningPanel(Renderer& r) : r_(r) {}

        /// 渲染采矿会话状态与采矿等级。
        void render_status(const MiningView& mining) const;

        /// 渲染本次采矿产出入库反馈（对应原 grant_mining 输出）。
        void render_produce(const std::vector<MiningEventView>& results) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view