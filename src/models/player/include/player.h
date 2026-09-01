#pragma once

// ==========================================================================
// 【占位桩】玩家属性 Model —— 由认领"玩家属性系统"的同事实现。
// 此处仅提供 tool/weather 模块需要的接口骨架，保证依赖方可编译。
// 命名空间按项目 mud 风格统一，内部实现为最简桩，待队友替换。
// ==========================================================================

namespace mud
{
    class Player
    {
    public:
        Player() = default;

        // 金币（仅供 tool 升级/修复使用）
        bool spend_gold(int amount);
        void add_gold(int amount);
        int gold() const;

        // TODO: 由队友补齐其余接口（饱食度/经验/等级/状态）

    private:
        int gold_ = 50;
    };

    // ---- 桩内部实现（最简，可编译可运行）----
    inline bool Player::spend_gold(int amount)
    {
        if (gold_ < amount) return false;
        gold_ -= amount;
        return true;
    }

    inline void Player::add_gold(int amount) { gold_ += amount; }

    inline int Player::gold() const { return gold_; }
}