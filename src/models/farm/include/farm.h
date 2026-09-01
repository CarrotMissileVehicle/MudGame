#pragma once

// ==========================================================================
// 【占位桩】农田 Model —— 由认领"种菜系统"的同事实现。
// 此处仅提供 weather 自动浇水需要的接口骨架。
// ==========================================================================

#include <cstddef>

namespace mud
{
    class Farm
    {
    public:
        Farm() = default;

        // 对已种植但未浇水的地块统一浇水（雨天自动调用）
        void auto_water();

        // TODO: 由队友补齐其余接口（地块/播种/收获/生长...）

    private:
        std::size_t plot_count_ = 0;
    };

    // ---- 桩内部实现（最简）----
    inline void Farm::auto_water() {}
}