//
// Created by z2996 on 2026/8/25.
//

#include <limits>
#include <utility>

#include "../include/Object.h"

namespace
{
    // 数量下限统一钳制：堆叠数量最小为 1
    int clamp_min_one(int value)
    {
        return value < 1 ? 1 : value;
    }
}

Object::Object(int sellingPrice, int buyingPrice)
        : sellingPrice(sellingPrice), buyingPrice(buyingPrice) {}

Object::Object(std::string name, std::string description, int health,  int sellingPrice, int buyingPrice) {
    this->name = std::move(name);
    this->description = std::move(description);
    this->health = health;
    this->sellingPrice = sellingPrice;
    this->buyingPrice = buyingPrice;
}

std::string Object::GetName() const {
    return name;
}

std::string Object::GetDescription() const {
    return description;
}

int Object::GetHealth() const {
    return health;
}

int Object::GetSellingPrice() const {
    return sellingPrice;
}

int Object::GetBuyingPrice() const {
    return buyingPrice;
}

int Object::GetQuantity() const {
    return quantity;
}

void Object::SetQuantity(int value) {
    quantity = clamp_min_one(value);
}

void Object::AddQuantity(int delta) {
    // 正增量做饱和加法，防止接近 INT_MAX 时溢出 UB
    if (delta > 0 && quantity > std::numeric_limits<int>::max() - delta)
        quantity = std::numeric_limits<int>::max();
    else
        quantity += delta;
    quantity = clamp_min_one(quantity);
}

void Object::Broke() {
    health -= BrokenStep;
    if (health < 0) health = 0;
}

void Object::Repair(int num) {
    if (num <= 0) return; // 拒绝非正值，避免 health 被反向削减
    health += num;
}

Object::~Object() = default;
