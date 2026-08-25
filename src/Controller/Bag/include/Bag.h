//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_BAG_H
#define MUDGAME_BAG_H
#include <string>
#include <vector>
#include "../../Objects/include/Object.h"

class Bag {
public:
    [[nodiscard]] const std::vector<std::string> GetAllObjectName() const;
    [[nodiscard]] const std::vector<std::string> GetDescription() const;

private:
    std::vector<Object*> objects;
};


#endif //MUDGAME_BAG_H