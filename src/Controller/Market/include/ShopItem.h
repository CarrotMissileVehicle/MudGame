#ifndef MUDGAME_SHOP_ITEM_H
#define MUDGAME_SHOP_ITEM_H

#include "Object.h"

// 商店中的单个可售物品条目。
// 价格一律取自物品自身，不再另设。
class ShopItem
{
public:
    ShopItem(Object* item = nullptr);

    Object* getItem() const;

private:
    Object* item;           // 物品实例
};

#endif //MUDGAME_SHOP_ITEM_H
