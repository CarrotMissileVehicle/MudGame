#include "../include/Shop.h"

#include <cstddef>

Shop::Shop(const std::string& id, const std::string& name)
        : id(id), name(name) {}

const std::string& Shop::getId() const { return id; }

const std::string& Shop::getName() const { return name; }

std::size_t Shop::itemCount() const { return items.size(); }

void Shop::addItem(ShopItem item) {
    items.push_back(item);
}

const ShopItem& Shop::getItem(std::size_t index) const { return items[index]; }

ShopItem& Shop::getItem(std::size_t index) { return items[index]; }

int Shop::getBuyPrice(Object* item) const {
    for (const ShopItem& entry : items) {
        if (entry.getItem() == item) return entry.getCurrentPrice();
    }
    return -1;
}
