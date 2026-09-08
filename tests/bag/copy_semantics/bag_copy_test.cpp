// DEF-002 回归：Bag 深拷贝/移动语义（修复"浅拷贝双重释放"缺陷后的行为锁）。
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

// 拷贝构造：副本与原包互不影响（深拷贝）
TEST(BagCopy, CopyConstructionIsolatesItems)
{
    Bag a;
    a.AddObject(make_item("胡萝卜"));
    ASSERT_EQ(a.GetSize(), 1u);

    Bag b = a;                     // 深拷贝
    ASSERT_EQ(b.GetSize(), 1u);

    b.RemoveObject("胡萝卜", 1);
    EXPECT_EQ(b.GetSize(), 0u);
    EXPECT_EQ(a.GetSize(), 1u);    // 原包不受副本操作影响
    EXPECT_EQ(a.CountObject("胡萝卜"), 1);
}

// 拷贝赋值：旧内容释放 + 深拷贝替换，两侧互不影响
TEST(BagCopy, CopyAssignmentIsolatesItems)
{
    Bag a;
    a.AddObject(make_item("南瓜", 3));
    Bag b;
    b.AddObject(make_item("小白菜"));

    b = a;                         // b 旧物品被释放，替换为 a 的深拷贝
    EXPECT_EQ(b.GetSize(), 1u);
    EXPECT_EQ(b.CountObject("南瓜"), 3);
    EXPECT_EQ(a.CountObject("南瓜"), 3);

    a.RemoveObject("南瓜", 1);
    EXPECT_EQ(a.CountObject("南瓜"), 2);
    EXPECT_EQ(b.CountObject("南瓜"), 3);   // b 不受 a 后续操作影响
}

// 移动构造：所有权转移，被移出对象变空（不残留悬垂指针）
TEST(BagCopy, MoveConstructionTransfersOwnership)
{
    Bag a;
    a.AddObject(make_item("鲈鱼", 2));

    Bag b = std::move(a);
    EXPECT_EQ(b.CountObject("鲈鱼"), 2);
    EXPECT_EQ(a.GetSize(), 0u);    // 被移出后为空
}

// 移动赋值：目标旧内容释放，接管源对象
TEST(BagCopy, MoveAssignmentTransfersOwnership)
{
    Bag a;
    a.AddObject(make_item("帝王蟹"));
    Bag b;
    b.AddObject(make_item("旧物品"));

    b = std::move(a);
    EXPECT_EQ(b.CountObject("帝王蟹"), 1);
    EXPECT_EQ(a.GetSize(), 0u);
}

// 自赋值安全（拷贝/移动均不应破坏自身）
TEST(BagCopy, SelfAssignmentIsSafe)
{
    Bag a;
    a.AddObject(make_item("灵芝", 5));

    auto& ref = a;
    a = ref;                       // 拷贝自赋值
    EXPECT_EQ(a.CountObject("灵芝"), 5);

    a = std::move(ref);            // 移动自赋值
    EXPECT_EQ(a.CountObject("灵芝"), 5);
}
