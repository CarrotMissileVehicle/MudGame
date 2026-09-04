#include "../include/ShopItem.h"

ShopItem::ShopItem(Object* item)
        : item(item) {}

Object* ShopItem::getItem() const { return item; }
