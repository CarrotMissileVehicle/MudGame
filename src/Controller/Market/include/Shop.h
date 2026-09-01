#ifndef MUDGAME_SHOP_H
#define MUDGAME_SHOP_H

#include <string>
#include <vector>
#include "ShopItem.h"

// 集市中的一个商店
class Shop
{
public:
    Shop(const std::string& id, const std::string& name);

    const std::string& getId() const;
    const std::string& getName() const;

    // 商店物品列表
    std::size_t itemCount() const;
    void addItem(ShopItem item);
    const ShopItem& getItem(std::size_t index) const;
    ShopItem& getItem(std::size_t index);

    // 计算某物品的当日购买价（基础价 × 浮动系数），未找到返回 -1
    int getBuyPrice(Object* item, float factor = 1.0f) const;

private:
    std::string id;                 // 商店 ID
    std::string name;               // 商店名称
    std::vector<ShopItem> items;    // 售卖物品列表
};

#endif //MUDGAME_SHOP_H
