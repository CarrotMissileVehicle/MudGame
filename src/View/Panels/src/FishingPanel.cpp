/**
 * @file FishingPanel.cpp
 * @brief 钓鱼面板实现。
 */
#include "FishingPanel.h"

#include <sstream>

namespace mud::view
{
    namespace
    {
        /// 以流默认精度输出浮点（避免 std::to_string 的尾零：0.4 而非 0.400000）。
        std::string fmt_probability(float p)
        {
            std::ostringstream os;
            os << p;
            return os.str();
        }
    } // namespace

    void FishingPanel::render(const FishingView& fishing) const
    {
        r_.print("鱼池 " + std::to_string(fishing.pool.size()) + " 种鱼类。");
        for (const auto& f : fishing.pool)
            r_.print("  " + f.name + "（概率 " + fmt_probability(f.probability) + "）");
        r_.print(fishing.can_fish ? "天气适宜钓鱼。" : "今日不宜垂钓。");
    }
} // namespace mud::view