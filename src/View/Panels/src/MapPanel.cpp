/**
 * @file MapPanel.cpp
 * @brief 地图面板实现：以可视化的十字地图展示全部地点与当前位置。
 */
#include "MapPanel.h"

namespace mud::view
{
    namespace
    {
        // 地图中的单个地点单元格：当前位置以 ※ 标记，否则留一个空格占位
        std::string cell(const std::string& name, bool here)
        {
            return here ? ("※" + name) : (" " + name);
        }
    } // namespace

    void MapPanel::render(PositionCode position) const
    {
        r_.print("========== 地图 ==========");
        r_.print("             " + cell("小镇", position == AtTown));
        r_.print("               |");
        r_.print(" " + cell("农田", position == AtFarmland) +
                 " —— " + cell("小屋", position == AtHome) +
                 " —— " + cell("海岸", position == AtCoast));
        r_.print("               |");
        r_.print("             " + cell("矿洞", position == AtMine));
        r_.print("  （※ = 你当前所在位置）");
        r_.print("当前位置：" + PositionName(position));
        r_.print("==========================");
    }

    std::string MapPanel::PositionName(PositionCode code)
    {
        switch (code)
        {
            case AtHome:     return "小屋";
            case AtFarmland: return "农田";
            case AtCoast:    return "海岸";
            case AtMine:     return "矿洞";
            case AtTown:     return "小镇";
            default:         return "未知";
        }
    }
} // namespace mud::view