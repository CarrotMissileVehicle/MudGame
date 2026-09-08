//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_OBJECT_H
#define MUDGAME_OBJECT_H
#include <string>


class Object {
public:
    Object(std::string name, std::string description, int health, int sellingPrice, int buyingPrice);
    Object(int sellingPrice = 0, int buyingPrice = 0);

    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] std::string GetDescription() const;
    [[nodiscard]] int GetHealth() const;
    [[nodiscard]] int GetSellingPrice() const;
    [[nodiscard]] int GetBuyingPrice() const;
    [[nodiscard]] int GetQuantity() const;
    void SetQuantity(int value);
    void AddQuantity(int delta);

    void Broke();
    void Repair(int num);

    virtual ~Object();

private:
    const int BrokenStep = 10;

    int sellingPrice = 0;
    int buyingPrice = 0;
    std::string name;
    std::string description;
    int health = 0;
    int quantity = 1;   // 堆叠数量（默认 1 个）
    std::string repairObj;
};


#endif //MUDGAME_OBJECT_H
