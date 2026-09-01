//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_BAG_H
#define MUDGAME_BAG_H
#include <string>
#include <vector>
#include "../../../model/Objects/include/Object.h"

class Bag {
public:
    [[nodiscard]] const std::vector<std::string> GetAllObjectName() const;

    [[nodiscard]] const std::vector<std::string> GetDescription() const;

    template<typename T>
    T *TryGetObjByType() {
        for (const auto obj: objects) {
            auto temp = dynamic_cast<T *>(obj);
            if (temp != nullptr)
                return temp;
        }
        return nullptr;
    }

    template<typename T>
    std::vector<T *> *TryGetAllObjByType() {
        auto *temp = new std::vector<T *>;
        for (const auto obj: objects) {
            auto tempObj = dynamic_cast<T *>(obj);
            if (tempObj != nullptr)
                temp->push_back(tempObj);
        }
        return temp;
    }

private:
    std::vector<Object *> objects;
};


#endif //MUDGAME_BAG_H
