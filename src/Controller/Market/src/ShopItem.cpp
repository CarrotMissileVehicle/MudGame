#include "../include/ShopItem.h"

ShopItem::ShopItem(Object* item, int basePrice, int currentPrice)
        : item(item),
          basePrice(basePrice),
          currentPrice(currentPrice) {}

Object* ShopItem::getItem() const { return item; }

int ShopItem::getBasePrice() const { return basePrice; }

void ShopItem::setBasePrice(int price) { basePrice = price; }

int ShopItem::getCurrentPrice() const { return currentPrice; }

void ShopItem::setCurrentPrice(int price) { currentPrice = price; }
