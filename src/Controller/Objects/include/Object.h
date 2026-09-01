//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_OBJECT_H
#define MUDGAME_OBJECT_H


class Object {
public:
    Object(int sellingPrice = 0, int buyingPrice = 0);

    virtual ~Object();

    int getSellingPrice() const;
    int getBuyingPrice() const;

protected:
    int sellingPrice;
    int buyingPrice;
};


#endif //MUDGAME_OBJECT_H
