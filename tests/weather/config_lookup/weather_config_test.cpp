// DEF-006 回归：Weather 配置按 type 字段查表（消除枚举序-数组序隐式耦合）。
// 天气生成是随机的，这里通过"多轮生成 + 按 current_name 分派预期值"验证
// 查询函数与生成类型始终一致（错配即失败）。
#include <gtest/gtest.h>

#include <string>

#include "weather.h"

// 多轮生成后，各查询函数返回值与生成天气的配置一致
TEST(WeatherConfig, QueriesMatchGeneratedType)
{
    mud::weather::Weather w;
    for (int i = 0; i < 300; ++i)
    {
        w.generate_daily();
        const std::string name = w.current_name();
        if (name == "晴天")
        {
            EXPECT_TRUE(w.can_fish());
            EXPECT_TRUE(w.can_go_outside());
            EXPECT_FALSE(w.auto_water());
            EXPECT_DOUBLE_EQ(w.crop_loss_rate(), 0.00);
            EXPECT_DOUBLE_EQ(w.mining_exp_bonus(), 0.00);
            EXPECT_DOUBLE_EQ(w.fishing_penalty(), 0.00);
        }
        else if (name == "小雨")
        {
            EXPECT_TRUE(w.can_fish());
            EXPECT_TRUE(w.can_go_outside());
            EXPECT_TRUE(w.auto_water());
            EXPECT_DOUBLE_EQ(w.fishing_penalty(), 0.10);
            EXPECT_DOUBLE_EQ(w.crop_loss_rate(), 0.00);
        }
        else if (name == "阴天")
        {
            EXPECT_TRUE(w.can_fish());
            EXPECT_TRUE(w.can_go_outside());
            EXPECT_DOUBLE_EQ(w.mining_exp_bonus(), 0.20);
            EXPECT_FALSE(w.auto_water());
        }
        else if (name == "暴风雨")
        {
            EXPECT_FALSE(w.can_fish());
            EXPECT_TRUE(w.can_go_outside());
            EXPECT_DOUBLE_EQ(w.crop_loss_rate(), 0.10);
            EXPECT_FALSE(w.auto_water());
        }
        else if (name == "台风")
        {
            EXPECT_FALSE(w.can_fish());
            EXPECT_FALSE(w.can_go_outside());
            EXPECT_DOUBLE_EQ(w.crop_loss_rate(), 0.35);
        }
        else
        {
            FAIL() << "未知天气名：" << name;
        }
    }
}

// 生成 300 次应覆盖全部 5 种天气（概率最低的台风 5%，漏检概率 < 1e-7）
TEST(WeatherConfig, AllWeatherTypesObserved)
{
    mud::weather::Weather w;
    bool sunny = false, rain = false, cloudy = false, storm = false, typhoon = false;
    for (int i = 0; i < 300; ++i)
    {
        w.generate_daily();
        const std::string name = w.current_name();
        sunny  |= (name == "晴天");
        rain   |= (name == "小雨");
        cloudy |= (name == "阴天");
        storm  |= (name == "暴风雨");
        typhoon |= (name == "台风");
    }
    EXPECT_TRUE(sunny);
    EXPECT_TRUE(rain);
    EXPECT_TRUE(cloudy);
    EXPECT_TRUE(storm);
    EXPECT_TRUE(typhoon);
}

// 概率分布合理性：晴天（40%）出现频率应显著高于台风（5%）
TEST(WeatherConfig, SunnyMoreFrequentThanTyphoon)
{
    mud::weather::Weather w;
    int sunny = 0, typhoon = 0;
    for (int i = 0; i < 1000; ++i)
    {
        w.generate_daily();
        const std::string name = w.current_name();
        if (name == "晴天") ++sunny;
        if (name == "台风") ++typhoon;
    }
    EXPECT_GT(sunny, typhoon);
    EXPECT_GT(sunny, 250);   // 40% 期望 400 次，下限 250
    EXPECT_LT(typhoon, 150); // 5% 期望 50 次，上限 150
}
