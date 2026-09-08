#include "../include/Market.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>

namespace
{
    // DEF-012：thread_local 引擎收敛全局 rand（多线程调用安全）。
    std::mt19937& rng()
    {
        static thread_local std::mt19937 gen(std::random_device{}());
        return gen;
    }
}

Market::Market(int dayOfWeek, int dayOfMonth)
        : dayOfWeek(dayOfWeek), dayOfMonth(dayOfMonth) {}

Market::~Market() {}

int Market::getDayOfWeek() const { return dayOfWeek; }

int Market::getDayOfMonth() const { return dayOfMonth; }

bool Market::isProsperousDay() const {
    // 每周三（dayOfWeek == 3）为繁华集，所有物品售价 +20%
    return dayOfWeek == 3;
}

bool Market::isFestival() const {
    // 每月 1 日为节日集
    return dayOfMonth == 1;
}

void Market::onNewDay(int week, int day) {
    dayOfWeek = week;
    dayOfMonth = day;
    updateFluctuations();
}

void Market::onNewDay(const mud::time::GameDateTime& time) {
    // 由累计天数推出星期(1-7)与日期(1-30)
    const std::int64_t dayCount = time.total_minutes() / 1440;
    int week = static_cast<int>((dayCount % 7)) + 1;
    int day  = static_cast<int>((dayCount % 30)) + 1;
    onNewDay(week, day);
}

void Market::registerShop(const Shop& shop) {
    shops.push_back(shop);
}

std::size_t Market::shopCount() const { return shops.size(); }

Shop& Market::getShop(std::size_t index) { return shops[index]; }

const Shop& Market::getShop(std::size_t index) const { return shops[index]; }

Shop* Market::findShop(const std::string& id) {
    for (Shop& shop : shops) {
        if (shop.getId() == id) return &shop;
    }
    return nullptr;
}

Shop* Market::findShopByItem(Object* item) {
    if (item == nullptr) return nullptr;
    for (Shop& shop : shops) {
        for (std::size_t i = 0; i < shop.itemCount(); ++i) {
            if (shop.getItem(i).getItem() == item) return &shop;
        }
    }
    return nullptr;
}

int Market::updateFluctuations() {
    // 收集所有在售物品
    std::vector<Object*> candidates;
    for (const Shop& shop : shops) {
        for (std::size_t i = 0; i < shop.itemCount(); ++i) {
            Object* item = shop.getItem(i).getItem();
            if (item != nullptr) candidates.push_back(item);
        }
    }
    if (candidates.empty()) return 0;

    fluctuations.clear();

    // 每天选择 2-3 种商品浮动 ±30%
    int count = 2 + std::uniform_int_distribution<int>(0, 1)(rng());      // 2 或 3
    count = std::min(count, static_cast<int>(candidates.size()));

    for (int i = 0; i < count; ++i) {
        int index = std::uniform_int_distribution<int>(
            0, static_cast<int>(candidates.size()) - 1)(rng());
        Object* item = candidates[index];
        // 偏移 [-0.30, 0.30]
        float offset = (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng()) - 0.5f) * 0.6f;
        float factor = 1.0f + offset;
        fluctuations[item] = factor;
    }

    return count;
}

float Market::getFluctuation(Object* item) const {
    if (item == nullptr) return 1.0f;
    auto it = fluctuations.find(item);
    if (it == fluctuations.end()) return 1.0f;
    return it->second;
}

float Market::normalSellPrice(Object* item) const {
    if (item == nullptr) return 0.0f;
    float price = static_cast<float>(item->GetSellingPrice());
    return price * getFluctuation(item);
}

int Market::getSellPrice(Object* item) const {
    if (item == nullptr) return 0;
    float price = normalSellPrice(item);
    if (isProsperousDay()) price *= 1.2f;   // 繁华集 +20%
    return static_cast<int>(price);
}

int Market::getBuyPrice(const std::string& shopId, Object* item) const {
    const Shop* shop = nullptr;
    for (const Shop& s : shops) {
        if (s.getId() == shopId) { shop = &s; break; }
    }
    if (shop == nullptr || item == nullptr) return 0;

    int base = shop->getBuyPrice(item);
    if (base < 0) return 0;

    float price = static_cast<float>(base) * getFluctuation(item);
    return static_cast<int>(price);
}

bool Market::buy(const std::string& shopId, Object* item,
                 int count, long long& gold) {
    if (count <= 0) return false;
    int price = getBuyPrice(shopId, item);
    if (price <= 0) return false;
    // count 可能较大（万分位计 → 千余），用 64 位算总额避免 int 溢出
    const long long total = static_cast<long long>(price) * count;
    if (gold < total) return false;   // 金币不足
    gold -= total;                    // DEF-007：全程 64 位回写，不再截断
    return true;
}

long long Market::sell(Object* item, int count, long long& gold) {
     if (item == nullptr || count <= 0) return 0;
    int price = getSellPrice(item);
    if (price <= 0) return 0;
    const long long total = static_cast<long long>(price) * count;
    gold += total;                    // DEF-007：全程 64 位回写，不再截断
    return total;
}
