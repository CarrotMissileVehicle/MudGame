//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_OBJECT_H
#define MUDGAME_OBJECT_H
#include <string>


class Object {
public:
    Object(std::string name, std::string description, int health, int sellingPrice, int buyingPrice);

    [[nodiscard]] std::string GetName() const;

    [[nodiscard]] std::string GetDescription() const;

    [[nodiscard]] int GetHealth() const;

    void Broke();

    void Repair(int num);

    virtual ~Object();

private:
    const int BrokenStep = 10;

    int sellingPrice;
    int buyingPrice;
    std::string name;
    std::string description;
    int health;
    std::string repairObj;
};


#endif //MUDGAME_OBJECT_H
