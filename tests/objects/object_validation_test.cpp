/**
 * @file object_validation_test.cpp
 * @brief Object 数值校验测试：Repair 拒绝非正值、数量下限钳制与饱和加法。
 */
#include <gtest/gtest.h>

#include <limits>

#include "Object.h"

TEST(ObjectValidationTest, RepairRejectsNonPositive)
{
    Object obj("锄头", "工具", 30, 10, 20);
    obj.Broke(); // 耐久 30 → 20

    obj.Repair(0);
    EXPECT_EQ(obj.GetHealth(), 20); // 无效果
    obj.Repair(-5);
    EXPECT_EQ(obj.GetHealth(), 20); // 负值不得反向削减
    obj.Repair(10);
    EXPECT_EQ(obj.GetHealth(), 30);
}

TEST(ObjectValidationTest, SetQuantityClampsToAtLeastOne)
{
    Object obj("胡萝卜", "蔬菜", 0, 5, 3);
    obj.SetQuantity(0);
    EXPECT_EQ(obj.GetQuantity(), 1);
    obj.SetQuantity(-100);
    EXPECT_EQ(obj.GetQuantity(), 1);
    obj.SetQuantity(7);
    EXPECT_EQ(obj.GetQuantity(), 7);
}

TEST(ObjectValidationTest, AddQuantitySaturatesOnOverflow)
{
    Object obj("胡萝卜", "蔬菜", 0, 5, 3);
    obj.SetQuantity(std::numeric_limits<int>::max() - 5);
    obj.AddQuantity(100); // 溢出 → 饱和到 INT_MAX，不产生 UB
    EXPECT_EQ(obj.GetQuantity(), std::numeric_limits<int>::max());
}

TEST(ObjectValidationTest, AddQuantityNegativeClampsToOne)
{
    Object obj("胡萝卜", "蔬菜", 0, 5, 3);
    obj.SetQuantity(5);
    obj.AddQuantity(-100);
    EXPECT_EQ(obj.GetQuantity(), 1);
}
