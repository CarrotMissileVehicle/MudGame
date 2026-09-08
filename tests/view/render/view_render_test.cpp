// View 层渲染测试：StringRenderer 捕获输出 + 面板渲染行为锁。
// 验证补齐的 Renderer.h / dto.h / view_primitives.h 与既有面板组装正确。
#include <gtest/gtest.h>

#include <string>

#include "Renderer.h"
#include "TimePanel.h"
#include "StatusPanel.h"
#include "MessagePanel.h"
#include "view_primitives.h"

// StringRenderer：print 追加换行、print_raw 原样
TEST(ViewRenderer, StringRendererCapturesLines)
{
    mud::view::StringRenderer r;
    r.print("hello");
    r.print_raw("world");
    r.print("!");

    EXPECT_EQ(r.text(), "hello\nworld!\n");
}

// 渲染原语：分隔线 / 键值对 / 时间格式化
TEST(ViewPrimitives, SeparatorAndPairFormat)
{
    mud::view::StringRenderer r;
    mud::view::print_separator(r, 5, '-');
    mud::view::print_pair(r, "金币", "200");

    EXPECT_EQ(r.text(), "-----\n金币: 200\n");
}

// 时间格式化：时分补零
TEST(ViewPrimitives, FormatTimePadsHourMinute)
{
    const mud::time::GameDateTime t{0, 1, 1, 8, 5};
    EXPECT_EQ(mud::view::format_time(t), "0年1月1日 08:05");

    const mud::time::GameDateTime t2{12, 11, 23, 0, 0};
    EXPECT_EQ(mud::view::format_time(t2), "12年11月23日 00:00");
}

// TimePanel：渲染时间与倍率
TEST(ViewPanels, TimePanelRenders)
{
    mud::view::StringRenderer r;
    mud::view::TimePanel panel(r);

    mud::view::TimeView t;
    t.year = 0; t.month = 1; t.day = 1; t.hour = 8; t.minute = 0;
    panel.render(t);

    panel.render_scale(2.0);

    EXPECT_NE(r.text().find("0年1月1日 08:00"), std::string::npos);
    EXPECT_NE(r.text().find("时间倍率设为 2"), std::string::npos);
}

// MessagePanel：逐行渲染
TEST(ViewPanels, MessagePanelRendersEachLine)
{
    mud::view::StringRenderer r;
    mud::view::MessagePanel panel(r);

    panel.render(mud::view::MessageLine{"第一行", "第二行"});
    EXPECT_EQ(r.text(), "第一行\n第二行\n");
}

// StatusPanel：完整玩家状态渲染（位置/状态/经验/背包）
TEST(ViewPanels, StatusPanelRendersPlayerStatus)
{
    mud::view::StringRenderer r;
    mud::view::StatusPanel panel(r);

    mud::view::PlayerStatus s;
    s.position = AtTown;
    s.state = Waiting;
    s.satiety = 80;
    s.maxSatiety = 100;
    s.farmingExp = 12;
    s.fishExp = 3;
    s.mineExp = 45;
    s.bagItems = {"胡萝卜 x4", "南瓜"};
    panel.render(s);

    const std::string& out = r.text();
    EXPECT_NE(out.find("Town"), std::string::npos);
    EXPECT_NE(out.find("Waiting"), std::string::npos);
    EXPECT_NE(out.find("80 / 100"), std::string::npos);
    EXPECT_NE(out.find("胡萝卜 x4"), std::string::npos);
    EXPECT_NE(out.find("Exp - Mining:  45"), std::string::npos);   // 两空格对齐
}

// DTO 聚合：GameSnapshot 默认构造可编译、字段可写（组合根装配通道）
TEST(ViewDto, SnapshotAggregatesAllViews)
{
    mud::view::GameSnapshot snap;
    snap.time.hour = 8;
    snap.weather.weather = "晴天";
    snap.player.satiety = 100;
    snap.farm.plots.push_back({});
    snap.market.gold = 200;
    snap.tools.tools.push_back({});
    snap.mining.is_mining = false;
    snap.fishing.can_fish = true;
    snap.messages.push_back("ok");

    EXPECT_EQ(snap.time.hour, 8);
    EXPECT_EQ(snap.market.gold, 200);
    ASSERT_EQ(snap.messages.size(), 1u);
}
