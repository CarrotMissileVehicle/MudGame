/**
 * @file panel_render_test.cpp
 * @brief View 面板渲染测试：用 StringRenderer 捕获输出并断言关键内容。
 *
 * 覆盖：TimePanel / StatusPanel / FarmPanel / FishingPanel / MarketPanel /
 * ToolsPanel / MiningPanel / WeatherPanel / MessagePanel / HelpScreen / TerminalView。
 * 同时验证 View 层"接收 DTO 渲染"契约（不触碰业务）。
 */
#include <gtest/gtest.h>

#include "Renderer.h"
#include "dto.h"
#include "TerminalView.h"

using namespace mud::view;

TEST(ViewPanelTest, TimePanelRendersFormattedTime)
{
    StringRenderer r;
    TimePanel panel(r);
    TimeView t;
    t.year = 0; t.month = 1; t.day = 1; t.hour = 8; t.minute = 5;
    panel.render(t);
    EXPECT_NE(r.text().find("现在时间：0年1月1日 08:05"), std::string::npos);
}

TEST(ViewPanelTest, StatusPanelRendersPlayerFields)
{
    StringRenderer r;
    StatusPanel panel(r);
    PlayerStatus s;
    s.position = AtFarmland;
    s.state = Waiting;
    s.satiety = 70;
    s.maxSatiety = 100;
    s.farmingExp = 12;
    s.fishExp = 0;
    s.mineExp = 0;
    s.bagItems = {"小白菜种子", "普通肥料"};
    panel.render(s);
    const std::string out = r.text();
    EXPECT_NE(out.find("Farmland"), std::string::npos);
    EXPECT_NE(out.find("Satiety:    70 / 100"), std::string::npos);
    EXPECT_NE(out.find("Exp - Farming: 12"), std::string::npos);
    EXPECT_NE(out.find("1. 小白菜种子"), std::string::npos);
    EXPECT_NE(out.find("2. 普通肥料"), std::string::npos);
}

TEST(ViewPanelTest, StatusPanelEmptyBagShowsEmpty)
{
    StringRenderer r;
    StatusPanel panel(r);
    PlayerStatus s{};
    panel.render(s);
    EXPECT_NE(r.text().find("(empty)"), std::string::npos);
}

TEST(ViewPanelTest, FarmPanelRendersPlots)
{
    StringRenderer r;
    FarmPanel panel(r);
    FarmView f;
    PlotView empty;                       // 空地
    empty.index = 0;
    PlotView planted;                     // 种植中
    planted.index = 1;
    planted.occupied = true;
    planted.crop_name = "小白菜";
    planted.growth_stage = 1;
    planted.growth_max = 2;
    planted.watered = true;
    f.plots = {empty, planted};
    panel.render(f);
    const std::string out = r.text();
    EXPECT_NE(out.find("[0] 空地"), std::string::npos);
    EXPECT_NE(out.find("[1] 小白菜（生长 1/2，已浇水）"), std::string::npos);
}

TEST(ViewPanelTest, FishingPanelRendersPool)
{
    StringRenderer r;
    FishingPanel panel(r);
    FishingView f;
    f.can_fish = true;
    f.pool = {{"小鲫鱼", 0.40f}, {"草鱼", 0.30f}};
    panel.render(f);
    const std::string out = r.text();
    EXPECT_NE(out.find("鱼池 2 种鱼类。"), std::string::npos);
    EXPECT_NE(out.find("小鲫鱼"), std::string::npos);
    EXPECT_NE(out.find("天气适宜钓鱼。"), std::string::npos);
}

TEST(ViewPanelTest, MarketPanelRendersShops)
{
    StringRenderer r;
    MarketPanel panel(r);
    MarketView m;
    m.gold = 200;
    m.day_of_week = 3;
    m.prosperous = true;
    ShopView shop;
    shop.id = "seed";
    shop.name = "种子商店";
    shop.items = {{"小白菜种子", 5, 8}};
    m.shops = {shop};
    panel.render(m);
    const std::string out = r.text();
    EXPECT_NE(out.find("金币：200"), std::string::npos);
    EXPECT_NE(out.find("集市日：周3（繁华集）"), std::string::npos);
    EXPECT_NE(out.find("[seed] 种子商店"), std::string::npos);
    EXPECT_NE(out.find("小白菜种子：买 5 / 卖 8"), std::string::npos);
}

TEST(ViewPanelTest, ToolsPanelRendersTools)
{
    StringRenderer r;
    ToolsPanel panel(r);
    ToolsView t;
    t.tools = {
        {"锄头", 45, 1, false},
        {"矿镐", 0, 1, true},
    };
    panel.render(t);
    const std::string out = r.text();
    EXPECT_NE(out.find("锄头：耐久 45，等级 1"), std::string::npos);
    EXPECT_NE(out.find("矿镐：耐久 0，等级 1 [已损坏]"), std::string::npos);
}

TEST(ViewPanelTest, MiningPanelRendersStatusAndProduce)
{
    StringRenderer r;
    MiningPanel panel(r);
    MiningView m;
    m.is_mining = true;
    m.layer = 2;
    m.start_time = mud::time::GameDateTime{0, 1, 1, 8, 0};
    m.mining_level = 3;
    panel.render_status(m);
    EXPECT_NE(r.text().find("状态：采矿中（层 2，自 0年1月1日 08:00 开始）"),
              std::string::npos);
    EXPECT_NE(r.text().find("采矿等级：3"), std::string::npos);

    r.clear();
    panel.render_produce({{"黯铁矿", 2, 10}});
    EXPECT_NE(r.text().find("  获得 黯铁矿 x2 (+10 采矿经验)"), std::string::npos);
}

TEST(ViewPanelTest, WeatherPanelRendersWeather)
{
    StringRenderer r;
    WeatherPanel panel(r);
    WeatherView w;
    w.weather = "小雨";
    w.auto_water = true;
    w.crop_loss_rate = 0.0;
    w.events = {"自动浇水"};
    panel.render(w);
    const std::string out = r.text();
    EXPECT_NE(out.find("今日天气：小雨"), std::string::npos);
    EXPECT_NE(out.find("自动浇水：是"), std::string::npos);
    EXPECT_NE(out.find("今日事件：自动浇水 "), std::string::npos);
}

TEST(ViewPanelTest, MessagePanelRendersLines)
{
    StringRenderer r;
    MessagePanel panel(r);
    panel.render({"移动成功。", "已播种 小白菜。"});
    const std::string out = r.text();
    EXPECT_NE(out.find("移动成功。"), std::string::npos);
    EXPECT_NE(out.find("已播种 小白菜。"), std::string::npos);
}

// MapPanel 可视化地图：所有地点都出现，当前位置带 ※ 标记
TEST(ViewPanelTest, MapPanelRendersVisualMapWithPosition)
{
    StringRenderer r;
    MapPanel panel(r);
    panel.render(AtHome); // 当前位置：小屋
    const std::string out = r.text();
    EXPECT_NE(out.find("地图"), std::string::npos);
    EXPECT_NE(out.find("※小屋"), std::string::npos); // 当前地点标记
    EXPECT_NE(out.find("农田"), std::string::npos);
    EXPECT_NE(out.find("小镇"), std::string::npos);
    EXPECT_NE(out.find("海岸"), std::string::npos);
    EXPECT_NE(out.find("矿洞"), std::string::npos);
    EXPECT_NE(out.find("当前位置：小屋"), std::string::npos);
}

// MapPanel 非当前位置不显示 ※ 标记
TEST(ViewPanelTest, MapPanelMarksOnlyCurrentPosition)
{
    StringRenderer r;
    MapPanel panel(r);
    panel.render(AtFarmland); // 当前位置：农田
    const std::string out = r.text();
    EXPECT_NE(out.find("※农田"), std::string::npos);
    // 其它地点不应带 ※ 前缀
    EXPECT_EQ(out.find("※小屋"), std::string::npos);
    EXPECT_EQ(out.find("※小镇"), std::string::npos);
}

TEST(ViewPanelTest, HelpScreenRendersRawText)
{
    StringRenderer r;
    HelpScreen panel(r);
    panel.render("help 显示帮助\nquit 退出游戏");
    const std::string out = r.text();
    EXPECT_NE(out.find("help 显示帮助"), std::string::npos);
    EXPECT_NE(out.find("quit 退出游戏"), std::string::npos);
}

TEST(ViewPanelTest, TerminalViewRenderAllComposesPanels)
{
    StringRenderer r;
    TerminalView tv(r);
    GameSnapshot snap;
    snap.messages = {"欢迎来到胡萝卜山谷。"};
    snap.time.year = 0; snap.time.month = 1; snap.time.day = 1;
    snap.time.hour = 8; snap.time.minute = 0;
    snap.weather.weather = "晴天";
    snap.player.position = AtTown;
    snap.player.state = Waiting;
    snap.player.satiety = 100;
    snap.player.maxSatiety = 100;
    snap.mining.is_mining = false;
    snap.mining.mining_level = 1;
    snap.fishing.can_fish = true;
    tv.render_all(snap);
    const std::string out = r.text();
    EXPECT_NE(out.find("欢迎来到胡萝卜山谷。"), std::string::npos);
    EXPECT_NE(out.find("现在时间：0年1月1日 08:00"), std::string::npos);
    EXPECT_NE(out.find("今日天气：晴天"), std::string::npos);
}

TEST(ViewPanelTest, PromptUsesPrintRawWithoutNewline)
{
    StringRenderer r;
    StdoutRenderer unused; // 仅验证 StringRenderer 侧行为
    (void)unused;
    print_prompt(r);
    EXPECT_EQ(r.text(), "> ");
}

TEST(ViewPanelTest, BlacksmithPanelRendersRepairInfo)
{
    StringRenderer r;
    BlacksmithPanel panel(r);
    BlacksmithView b;
    b.gold = 120;
    b.tools = {
        {"锄头", 30, 50, false, 20, "黯铁矿", 2, 1},
        {"矿镐", 0, 20, true, 80, "黯铁矿", 2, 0},
    };
    panel.render(b);
    const std::string out = r.text();
    EXPECT_NE(out.find("铁匠铺——工具修复"), std::string::npos);
    EXPECT_NE(out.find("金币：120"), std::string::npos);
    EXPECT_NE(out.find("锄头：耐久 30/50，修复需 黯铁矿 x2（持有 1）或金币 20"),
              std::string::npos);
    EXPECT_NE(out.find("矿镐：耐久 0/20 [已损坏]，修复需 黯铁矿 x2（持有 0）或金币 80"),
              std::string::npos);
}

TEST(ViewPanelTest, InputHintPrintInputFormatPrefix)
{
    StringRenderer r;
    print_input_hint(r, "move.<方向> 移动到地点");
    EXPECT_EQ(r.text(), "输入格式：move.<方向> 移动到地点\n");
}