//
// Created by z2996 on 2026/8/25.
//

#include "../include/Object.h"

Object::Object(int sellingPrice, int buyingPrice)
        : sellingPrice(sellingPrice), buyingPrice(buyingPrice) {}

Object::~Object() {}

int Object::getSellingPrice() const { return sellingPrice; }

int Object::getBuyingPrice() const { return buyingPrice; }
