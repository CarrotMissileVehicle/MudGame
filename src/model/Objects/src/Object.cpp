//
// Created by z2996 on 2026/8/25.
//

#include <utility>

#include "../include/Object.h"

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

void Object::Broke() {
    health -= BrokenStep;
    if (health < 0) health = 0;
}

void Object::Repair(int num) {
    health += num;
}

Object::~Object() = default;
