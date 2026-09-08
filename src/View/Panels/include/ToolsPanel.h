/**
 * @file ToolsPanel.h
 * @brief 工具面板：渲染三件工具的耐久、等级与损坏状态（收编 main 的 tools.status）。
 */
#pragma once

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class ToolsPanel
    {
    public:
        explicit ToolsPanel(Renderer& r) : r_(r) {}

        /// 渲染全部工具状态。
        void render(const ToolsView& tools) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view