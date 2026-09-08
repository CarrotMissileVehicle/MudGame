/**
 * @file TimePanel.cpp
 * @brief 时间面板实现。
 */
#include "TimePanel.h"

#include <sstream>

#include "view_primitives.h"

namespace mud::view
{
    void TimePanel::render(const TimeView& t) const
    {
        r_.print("现在时间：" + format_time(mud::time::GameDateTime{t.year, static_cast<unsigned>(t.month), static_cast<unsigned>(t.day), static_cast<unsigned>(t.hour), static_cast<unsigned>(t.minute)}));
    }

    void TimePanel::render_scale(double factor) const
    {
        std::ostringstream os;
        os << factor; // 120 / 0.5 等，避免 std::to_string 的尾零与 %.0f 的取整截断
        r_.print("时间倍率设为 " + os.str() + "。");
    }
} // namespace mud::view