#ifndef MUDGAME_SHOP_ITEM_H
#define MUDGAME_SHOP_ITEM_H

#include "Object.h"

// 商店中的单个可售物品条目。
// 每个条目记录物品实例、基础价格与当前价格。
class ShopItem
{
public:
    ShopItem(Object* item = nullptr, int basePrice = 0,
             int currentPrice = 0);

    Object* getItem() const;

    int getBasePrice() const;
    void setBasePrice(int price);

    int getCurrentPrice() const;
    void setCurrentPrice(int price);

private:
    Object* item;           // 物品实例
    int basePrice;          // 基础价格
    int currentPrice;       // 当前价格（基础价 × 浮动系数）
};

#endif //MUDGAME_SHOP_ITEM_H
