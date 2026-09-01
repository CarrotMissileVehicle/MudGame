#pragma once

// ==========================================================================
// 【占位桩】背包 Model —— 由认领"背包系统"的同事实现。
// 此处仅提供 tool 升级/修复需要的接口骨架（按 item_id 判断/扣除材料）。
// ==========================================================================

#include <string>

namespace mud
{
    class Inventory
    {
    public:
        Inventory() = default;

        bool has_item(const std::string& item_id, int count) const;
        void remove_item(const std::string& item_id, int count);
        void add_item(const std::string& item_id, int count);

        // TODO: 由队友补齐其余接口（不限容量存储/查询）

    private:
        int placeholder_ = 0;
    };

    // ---- 桩内部实现（最简：视为始终有足够材料，可运行）----
    inline bool Inventory::has_item(const std::string&, int) const { return true; }

    inline void Inventory::remove_item(const std::string&, int) {}

    inline void Inventory::add_item(const std::string&, int) {}
}