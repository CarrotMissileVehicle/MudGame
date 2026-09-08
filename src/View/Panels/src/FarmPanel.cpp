/**
 * @file FarmPanel.cpp
 * @brief 农田面板实现。
 */
#include "FarmPanel.h"

namespace mud::view
{
    void FarmPanel::render(const FarmView& farm) const
    {
        for (const auto& plot : farm.plots)
        {
            std::string line = "[" + std::to_string(plot.index) + "] ";
            if (!plot.occupied)
            {
                line += "空地";
            }
            else
            {
                line += plot.crop_name;
                line += "（生长 " + std::to_string(plot.growth_stage) + "/" +
                        std::to_string(plot.growth_max);
                line += plot.watered ? "，已浇水）" : "，缺水）";
            }
            r_.print(line);
        }
    }
} // namespace mud::view