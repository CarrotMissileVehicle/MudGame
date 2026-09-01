#ifndef FOOD_H
#define FOOD_H


#include "Object.h"

class Food: public Object
{
public:
    Food(int satiationRecovery = 0, int sellingPrice = 0, int buyingPrice = 0);
    ~Food() override;

    int getSatiationRecovery() const;

private:
    int satiationRecovery = 0;
};


#endif