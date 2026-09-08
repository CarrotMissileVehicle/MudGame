//
// Created by z2996 on 2026/8/25.
//

#include "../include/Bag.h"

#include <algorithm>

Bag::~Bag() {
    for (auto obj : objects) {
        delete obj;
    }
    objects.clear();
}

void Bag::AddObject(Object* obj) {
    if (obj == nullptr) return;
    // 堆叠逻辑：与已有同名物品合并数量，释放传入对象
    for (auto* existing : objects) {
        if (existing->GetName() == obj->GetName()) {
            existing->AddQuantity(obj->GetQuantity());
            delete obj;
            return;
        }
    }
    objects.push_back(obj);
}

void Bag::AddUnique(Object* obj) {
    if (obj != nullptr) {
        objects.push_back(obj);
    }
}

int Bag::RemoveObject(const std::string& name, int count) {
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if ((*it)->GetName() != name) continue;
        Object* obj = *it;
        const int have = obj->GetQuantity();
        const int take = std::min(count, have);
        if (take >= have) {
            delete obj;
            objects.erase(it);
        } else {
            obj->AddQuantity(-take);
        }
        return take;
    }
    return 0;
}

bool Bag::HasObject(const std::string& name) const {
    for (const auto obj : objects) {
        if (obj->GetName() == name) {
            return true;
        }
    }
    return false;
}

int Bag::CountObject(const std::string& name) const {
    int total = 0;
    for (const auto obj : objects) {
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
    for (const auto obj : objects) {
        names.push_back(obj->GetName());
    }
    return names;
}

const std::vector<std::string> Bag::GetStackedNames() const {
    std::vector<std::string> names;
    for (const auto obj : objects) {
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
    for (const auto obj : objects) {
        std::string d = obj->GetDescription();
        if (obj->GetQuantity() > 1) {
            d += " x" + std::to_string(obj->GetQuantity());
        }
        descriptions.push_back(d);
    }
    return descriptions;
}

