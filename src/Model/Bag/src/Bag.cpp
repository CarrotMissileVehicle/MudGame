//
// Created by z2996 on 2026/8/25.
//

#include "../include/Bag.h"

#include <algorithm>
#include <memory>
#include <utility>

Bag::~Bag() = default;   // unique_ptr 自动释放（H14）

// 深拷贝：为每件物品分配新对象，副本与原包互不影响
Bag::Bag(const Bag& other)
{
    objects.reserve(other.objects.size());
    for (const auto& obj : other.objects)
        objects.push_back(std::make_unique<Object>(*obj));
}

// copy-and-swap：构造副本失败时（分配抛出）this 保持不变，异常安全（H15）
Bag& Bag::operator=(const Bag& other)
{
    if (this == &other) return *this;
    Bag tmp(other);
    objects.swap(tmp.objects);
    return *this;
}

Bag::Bag(Bag&& other) noexcept = default;

Bag& Bag::operator=(Bag&& other) noexcept = default;

void Bag::AddObject(Object* obj) {
    if (obj == nullptr) return;
    // 立即接管所有权：后续任何抛出路径都由 unique_ptr 释放，不再泄漏（H14）
    std::unique_ptr<Object> holder(obj);
    // 堆叠逻辑：与已有同名物品合并数量，释放传入对象
    for (auto& existing : objects) {
        if (existing->GetName() == holder->GetName()) {
            existing->AddQuantity(holder->GetQuantity());
            return;
        }
    }
    objects.push_back(std::move(holder));
}

void Bag::AddUnique(Object* obj) {
    if (obj != nullptr) {
        objects.push_back(std::unique_ptr<Object>(obj));
    }
}

int Bag::RemoveObject(const std::string& name, int count) {
    // 拒绝非正数：负数会让 std::min 反向“增加”堆叠数量，破坏背包状态
    if (count <= 0) {
        return 0;
    }
    // 跨堆叠累计扣减（H9）：同名多堆叠（读档 AddUnique 还原）时依次扣减直到凑足 count
    int removed = 0;
    for (auto it = objects.begin(); it != objects.end() && removed < count; ) {
        if ((*it)->GetName() != name) {
            ++it;
            continue;
        }
        const int have = (*it)->GetQuantity();
        const int take = std::min(count - removed, have);
        if (take >= have) {
            it = objects.erase(it);
        } else {
            (*it)->AddQuantity(-take);
            ++it;
        }
        removed += take;
    }
    return removed;
}

bool Bag::HasObject(const std::string& name) const {
    for (const auto& obj : objects) {
        if (obj->GetName() == name) {
            return true;
        }
    }
    return false;
}

int Bag::CountObject(const std::string& name) const {
    int total = 0;
    for (const auto& obj : objects) {
        if (obj->GetName() == name) {
            total += obj->GetQuantity();
        }
    }
    return total;
}

size_t Bag::GetSize() const {
    return objects.size();
}

const std::vector<std::string> Bag::GetAllObjectName() const {
    std::vector<std::string> names;
    for (const auto& obj : objects) {
        names.push_back(obj->GetName());
    }
    return names;
}

const std::vector<std::string> Bag::GetStackedNames() const {
    std::vector<std::string> names;
    for (const auto& obj : objects) {
        if (obj->GetQuantity() > 1) {
            names.push_back(obj->GetName() + " x" + std::to_string(obj->GetQuantity()));
        } else {
            names.push_back(obj->GetName());
        }
    }
    return names;
}

const std::vector<std::string> Bag::GetDescription() const {
    std::vector<std::string> descriptions;
    for (const auto& obj : objects) {
        std::string d = obj->GetDescription();
        if (obj->GetQuantity() > 1) {
            d += " x" + std::to_string(obj->GetQuantity());
        }
        descriptions.push_back(d);
    }
    return descriptions;
}
