// 背包堆叠功能：入包合并、部分/全额扣减、跨堆叠计数与展示。
#include <gtest/gtest.h>

#include "Bag.h"
#include "Object.h"

namespace
{
    Object* make_item(const std::string& name, int qty = 1)
    {
        auto* obj = new Object(name, "测试物品", 0, 10, 5);
        obj->SetQuantity(qty);
        return obj;
    }
}

// 同名物品入包自动堆叠合并，数量累加
TEST(BagStack, AddObjectMergesSameName)
{
    Bag bag;
    bag.AddObject(make_item("胡萝卜"));
    bag.AddObject(make_item("胡萝卜", 3));
    bag.AddObject(make_item("南瓜"));

    EXPECT_EQ(bag.GetSize(), 2u);              // 两个堆叠：胡萝卜 x4、南瓜 x1
    EXPECT_EQ(bag.CountObject("胡萝卜"), 4);
    EXPECT_EQ(bag.CountObject("南瓜"), 1);
}

// AddUnique 不做堆叠合并（读档还原已聚合堆叠）
TEST(BagStack, AddUniqueSkipsMerging)
{
    Bag bag;
    bag.AddUnique(make_item("胡萝卜", 2));
    bag.AddUnique(make_item("胡萝卜", 3));

    EXPECT_EQ(bag.GetSize(), 2u);              // 两个独立堆叠
    EXPECT_EQ(bag.CountObject("胡萝卜"), 5);   // 跨堆叠计数正确
}

// 部分扣减：数量减到非零，堆叠保留
TEST(BagStack, RemoveObjectPartialKeepsStack)
{
    Bag bag;
    bag.AddObject(make_item("胡萝卜", 5));

    EXPECT_EQ(bag.RemoveObject("胡萝卜", 2), 2);
    EXPECT_EQ(bag.CountObject("胡萝卜"), 3);
    EXPECT_EQ(bag.GetSize(), 1u);
}

// 全额扣减：数量归零后堆叠删除
TEST(BagStack, RemoveObjectFullDeletesStack)
{
    Bag bag;
    bag.AddObject(make_item("胡萝卜", 5));
    bag.AddObject(make_item("南瓜"));

    EXPECT_EQ(bag.RemoveObject("胡萝卜", 5), 5);
    EXPECT_EQ(bag.GetSize(), 1u);
    EXPECT_FALSE(bag.HasObject("胡萝卜"));
    EXPECT_TRUE(bag.HasObject("南瓜"));
}

// 扣减数量超过持有量：按实际持有返回并删堆叠
TEST(BagStack, RemoveObjectOverCountReturnsHeld)
{
    Bag bag;
    bag.AddObject(make_item("鲈鱼", 2));

    EXPECT_EQ(bag.RemoveObject("鲈鱼", 10), 2);
    EXPECT_EQ(bag.GetSize(), 0u);
}

// 移除不存在的物品返回 0
TEST(BagStack, RemoveMissingObjectReturnsZero)
{
    Bag bag;
    EXPECT_EQ(bag.RemoveObject("帝王蟹", 1), 0);
    EXPECT_EQ(bag.GetSize(), 0u);
}

// 展示列表：数量大于 1 显示 "名称 xN"
TEST(BagStack, StackedNamesFormat)
{
    Bag bag;
    bag.AddObject(make_item("胡萝卜", 4));
    bag.AddObject(make_item("南瓜"));

    const auto names = bag.GetStackedNames();
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "胡萝卜 x4");
    EXPECT_EQ(names[1], "南瓜");
}
