/**
 * @file ToolsPanel.cpp
 * @brief 工具面板实现。
 */
#include "ToolsPanel.h"

namespace mud::view
{
    void ToolsPanel::render(const ToolsView& tools) const
    {
        for (const auto& t : tools.tools)
        {
            std::string line = t.name + "：耐久 " + std::to_string(t.durability) +
                               "，等级 " + std::to_string(t.level);
            if (t.broken) line += " [已损坏]";
            r_.print(line);
        }
    }
} // namespace mud::view