/**
 * @file bag_stack_test.cpp
 * @brief 背包堆叠逻辑测试。
 *
 * 覆盖：AddObject 同名合并、GetStackedNames 展示数量、RemoveObject 扣减、
 * 不同名物品各自成堆、AddUnique 读档还原不合并。
 */
#include <gtest/gtest.h>

#include "Bag.h"

TEST(BagStackTest, AddObjectStacksByName)
{
    Bag bag;
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    EXPECT_EQ(bag.GetSize(), 1u);          // 只保留一个堆叠
    EXPECT_EQ(bag.CountObject("小麦"), 3); // 总量叠加
    EXPECT_TRUE(bag.HasObject("小麦"));
}

TEST(BagStackTest, DisplayNamesAffixQuantityBeyondOne)
{
    Bag bag;
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("胡萝卜", "农作物", 0, 8, 5));
    const auto names = bag.GetStackedNames();
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "小麦 x2");   // 数量大于 1 附加 xN
    EXPECT_EQ(names[1], "胡萝卜");     // 单个不加后缀
}

TEST(BagStackTest, GetAllObjectNameDeduplicatesPerStack)
{
    Bag bag;
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    const auto names = bag.GetAllObjectName();
    ASSERT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "小麦");
}

TEST(BagStackTest, RemoveObjectDecrementsStack)
{
    Bag bag;
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    EXPECT_EQ(bag.RemoveObject("小麦", 2), 2); // 扣减 2
    EXPECT_EQ(bag.CountObject("小麦"), 1);
    EXPECT_EQ(bag.GetSize(), 1u);
    EXPECT_EQ(bag.RemoveObject("小麦", 5), 1); // 数量不足时清空该堆叠
    EXPECT_FALSE(bag.HasObject("小麦"));
    EXPECT_EQ(bag.GetSize(), 0u);
}

TEST(BagStackTest, RemoveDefaultCountOneKeepsOldSignature)
{
    Bag bag;
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    EXPECT_EQ(bag.RemoveObject("小麦"), 1); // 缺省移除 1 个
    EXPECT_EQ(bag.CountObject("小麦"), 1);
    EXPECT_EQ(bag.GetSize(), 1u);
}

TEST(BagStackTest, DifferentNamesStaySeparateStacks)
{
    Bag bag;
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("黯铁矿", "矿石", 0, 20, 0));
    EXPECT_EQ(bag.GetSize(), 2u);
    EXPECT_EQ(bag.CountObject("小麦"), 1);
    EXPECT_EQ(bag.CountObject("黯铁矿"), 1);
}

TEST(BagStackTest, AddUniqueDoesNotMergeStacks)
{
    Bag bag;
    bag.AddUnique(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddUnique(new Object("小麦", "农作物", 0, 5, 3));
    EXPECT_EQ(bag.GetSize(), 2u); // 读档还原不做合并
    EXPECT_EQ(bag.CountObject("小麦"), 2);
}

TEST(BagStackTest, CopyDeepCopiesStacks)
{
    Bag bag;
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("黯铁矿", "矿石", 0, 20, 0));

    Bag copy(bag);          // 复制构造
    EXPECT_EQ(copy.GetSize(), 2u);
    EXPECT_EQ(copy.CountObject("小麦"), 1);
    copy.RemoveObject("小麦");
    EXPECT_FALSE(copy.HasObject("小麦"));
    EXPECT_TRUE(bag.HasObject("小麦")); // 原包不受副本影响

    Bag assigned;
    assigned.AddObject(new Object("其他", "无关", 0, 1, 1));
    assigned = bag;         // 深拷贝赋值
    EXPECT_EQ(assigned.GetSize(), 2u);
    ASSERT_FALSE(assigned.GetObjects().empty());
    EXPECT_EQ(assigned.GetObjects()[0]->GetName(), "小麦");
    EXPECT_EQ(assigned.GetObjects()[1]->GetName(), "黯铁矿");
    // 修改副本不影响原包（深拷贝互不共享）
    assigned.RemoveObject("黯铁矿");
    EXPECT_TRUE(bag.HasObject("黯铁矿"));
}

TEST(BagStackTest, MoveTransfersOwnershipWithoutDoubleFree)
{
    Bag a;
    a.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    Bag b(std::move(a));
    EXPECT_EQ(b.CountObject("小麦"), 1);
    EXPECT_EQ(a.GetSize(), 0u); // 源包被清空，不再持有指针
    b.RemoveObject("小麦");     // 释放后不崩溃
}

TEST(BagStackTest, RemoveObjectRejectsNonPositiveCount)
{
    Bag bag;
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    bag.AddObject(new Object("小麦", "农作物", 0, 5, 3));
    EXPECT_EQ(bag.CountObject("小麦"), 3);
    // 零与负数计数：拒绝且不得改变背包任何状态
    EXPECT_EQ(bag.RemoveObject("小麦", 0), 0);
    EXPECT_EQ(bag.RemoveObject("小麦", -3), 0);
    EXPECT_EQ(bag.CountObject("小麦"), 3);
    EXPECT_EQ(bag.GetSize(), 1u);
    EXPECT_EQ(bag.RemoveObject("不存在", -1), 0);
    EXPECT_EQ(bag.GetSize(), 1u);
}

// H9：同名多堆叠（读档 AddUnique 还原场景）时 RemoveObject 必须跨堆叠累计扣减，
// 否则出售按 CountObject 入账、只扣首个堆叠，形成刷钱漏洞
TEST(BagStackTest, RemoveObjectSpansMultipleStacks)
{
    Bag bag;
    bag.AddUnique(new Object("小麦", "农作物", 0, 5, 3)); // 堆叠1: x1
    bag.AddUnique(new Object("小麦", "农作物", 0, 5, 3)); // 堆叠2: x1
    bag.AddUnique(new Object("小麦", "农作物", 0, 5, 3)); // 堆叠3: x1
    ASSERT_EQ(bag.GetSize(), 3u);
    ASSERT_EQ(bag.CountObject("小麦"), 3);

    // 一次扣 2：应跨堆叠清空两个堆叠
    EXPECT_EQ(bag.RemoveObject("小麦", 2), 2);
    EXPECT_EQ(bag.CountObject("小麦"), 1);
    EXPECT_EQ(bag.GetSize(), 1u);
    EXPECT_TRUE(bag.HasObject("小麦"));

    // 再扣 5（超过持有量）：清空剩余堆叠，返回实际移除量 1
    EXPECT_EQ(bag.RemoveObject("小麦", 5), 1);
    EXPECT_EQ(bag.CountObject("小麦"), 0);
    EXPECT_EQ(bag.GetSize(), 0u);
}

TEST(BagStackTest, AddObjectNullGuard)
{
    Bag bag;
    bag.AddObject(nullptr); // 空指针：忽略且不改变状态、不崩溃
    EXPECT_EQ(bag.GetSize(), 0u);
    bag.AddUnique(nullptr);
    EXPECT_EQ(bag.GetSize(), 0u);
}