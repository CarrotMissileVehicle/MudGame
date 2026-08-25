#pragma once              // 防止头文件被重复包含

#include <string>         // 字符串类型
#include <vector>         // 可变长数组(动态列表)类型
#include "event.h"        // 事件系统 Model
#include "weather.h"      // 天气 Model
#include "time_system.h"  // 依赖：时间系统（占位）
#include "farm.h"         // 依赖：农田（占位）

// ---- 类：天气与事件控制器 ----
// Controller 的角色：把(时间/天气/农田等)串起来协调，并给其他系统提供调用入口。
// 依赖通过构造函数"注入"（把别人的对象传进来用），这样团队协作更清晰。
class WeatherController {
public:   // 公开区：给其他人调用的接口
    // 构造函数：需要传入 TimeSystem 和 Farm 两个依赖的"引用(&)"。
    // 用了引用 → 我们持有的是对方那个对象本体，不是复制品。
    WeatherController(TimeSystem& time, Farm& farm);

    // 主循环每一帧都调用它：
    //   - 发现跨天(到新的一天6点) → 生成当天天气
    //   - 到早上8点 → 触发当天随机事件
    void update();

    // ---- 天气查询接口（往下都只是把 Model 的结果转发给调用方）----
    WeatherType getWeather() const;         // 当前天气类型
    std::string getWeatherName() const;     // 当前天气中文名
    bool canFish() const;                   // 能钓鱼吗
    bool canGoOutside() const;              // 能外出吗
    bool autoWater() const;                 // 是否自动浇水
    float cropLossRate() const;             // 当前天气减产比例
    float miningExpBonus() const;           // 采矿经验加成
    float fishingPenalty() const;           // 钓鱼减益

    // ---- 事件查询接口 ----
    bool hasEvent(EventType type) const;    // 今天是否触发某事件
    bool isTravelerActive() const;          // 旅行商人在场吗
    std::vector<std::string> todayEventNames() const;  // 今天触发了哪些事件名（给界面用）

    // 采矿时的随机事件判定（宝箱/塌方）
    void rollMiningEvent(bool& foundChest, bool& caveIn);

private:   // 私有区：只给自己内部调用的辅助方法
    // 检查是否跨天，跨了就生成天气。返回 true 表示确实跨天了
    bool checkNewDay();
    // 检查是否到早8点触发事件（同一天只触发一次）
    void checkEventTrigger();

    Weather weather_;       // 天气数据本体
    EventSystem events_;    // 事件数据本体
    TimeSystem& time_;      // 注入进来的时间系统(引用)
    Farm& farm_;            // 注入进来的农田(引用)

    int lastWeatherDay_;    // 记住"上次生成天气是哪天"，避免每天都重复生成
    int lastEventDay_;      // 记住"上次触发事件是哪天"，避免同一天重复触发
};