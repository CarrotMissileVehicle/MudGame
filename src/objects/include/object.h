/**
 * @file object.h
 * @brief 游戏物品基类（object）。
 *
 * 所有可交易/可拾取物品的抽象基类，携带买卖价格字段。
 */
//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_OBJECT_H
#define MUDGAME_OBJECT_H


/** @brief 游戏物品抽象基类。 */
class object {
public:
    object();

    virtual ~object();

protected:
    int sellingPrice; // 售价
    int buyingPrice;  // 买价
};


#endif //MUDGAME_OBJECT_H
