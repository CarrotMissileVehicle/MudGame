//
// Created by z2996 on 2026/8/25.
//

#include "../include/Bag.h"

Bag::~Bag() {
    for (auto obj : objects) {
        delete obj;
    }
    objects.clear();
}

void Bag::AddObject(Object* obj) {
    if (obj != nullptr) {
        objects.push_back(obj);
    }
}

void Bag::RemoveObject(const std::string& name) {
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if ((*it)->GetName() == name) {
            delete *it;
            objects.erase(it);
            return;
        }
    }
}

bool Bag::HasObject(const std::string& name) const {
    for (const auto obj : objects) {
        if (obj->GetName() == name) {
            return true;
        }
    }
    return false;
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

const std::vector<std::string> Bag::GetDescription() const {
    std::vector<std::string> descriptions;
    for (const auto obj : objects) {
        descriptions.push_back(obj->GetDescription());
    }
    return descriptions;
}

