/**
 * @file weather_Controller.cpp
 * @brief 天气与事件控制器实现。
 *
 * 实现主循环心跳的跨天天气生成与早 8 点事件触发，并转发天气/事件查询。
 */
#include "weather_Controller.h"   // 实现这个头文件里声明的方法
#include <vector>                 // 可变长数组类型

// ---- 构造函数实现 ----
// 用初始化列表把依赖(引用)和两个"记录用"整数设好。
// time_ / farm_ 是引用，必须用初始化列表绑定，不能在函数体里赋值。
WeatherController::WeatherController(mud::TimeService& time, Farm& farm)
    : time_(time), farm_(farm), lastWeatherDay_(0), lastEventDay_(0) {}

// ---- 每帧心跳：负责触发天气生成与事件判定 ----
void WeatherController::update() {
    checkNewDay();        // 1) 检查跨天→新一天生成天气
    checkEventTrigger();  // 2) 检查是否到早8点→触发事件
}

// ---- 检查跨天并生成天气 ----
bool WeatherController::checkNewDay() {
    // 时间系统说"今天"还是我们上次生成时的那天 → 说明没跨天，直接返回 false
    if (time_.day() == lastWeatherDay_) return false;
    // 还没到早上6点(比如凌晨3点) → 仍算前一夜，不生成新天气
    if (time_.hour() < 6) return false;

    lastWeatherDay_ = time_.day();   // 记住今天的日期
    weather_.generateDaily();        // 让天气对象按概率随机选一种当天天气

    // 如果是雨天：自动帮农田浇一次水（调用农田系统的接口）
    if (weather_.autoWater()) {
        farm_.autoWater();
    }
    return true;   // 表示确实跨天了
}

// ---- 检查早8点并触发事件 ----
void WeatherController::checkEventTrigger() {
    if (time_.day() == lastEventDay_) return;   // 今天触发过了，跳过
    if (time_.hour() != 8) return;              // 没到早8点，跳过

    lastEventDay_ = time_.day();   // 记住今天已触发，防止重复

    // ---- 下面两个值目前是占位，等队友的时间/农田系统就绪后替换 ----
    const bool neglectWater = false;  // 暂定"没有连续3天没浇水"
    const int dayOfWeek = 1;          // 暂定今天是周一

    events_.generateDailyEvents(dayOfWeek, neglectWater);  // 让事件对象按规则判定
}

// ---- 以下全是简单的"转发"：把 Model 的查询结果原样交给调用方 ----
WeatherType WeatherController::getWeather() const { return weather_.current(); }

std::string WeatherController::getWeatherName() const { return weather_.currentName(); }

bool WeatherController::canFish() const { return weather_.canFish(); }

bool WeatherController::canGoOutside() const { return weather_.canGoOutside(); }

bool WeatherController::autoWater() const { return weather_.autoWater(); }

float WeatherController::cropLossRate() const { return weather_.cropLossRate(); }

float WeatherController::miningExpBonus() const { return weather_.miningExpBonus(); }

float WeatherController::fishingPenalty() const { return weather_.fishingPenalty(); }

bool WeatherController::hasEvent(EventType type) const { return events_.hasEvent(type); }

bool WeatherController::isTravelerActive() const { return events_.isTravelerActive(); }

// 返回今天触发的所有"早8点类"事件的中文名，方便界面直接显示
std::vector<std::string> WeatherController::todayEventNames() const {
    std::vector<std::string> names;   // 建一个空的字符串列表
    // 遍历配置表，找出"今天已触发"且"属于早8点类"的事件
    for (int i = 0; i < kEventConfigCount; ++i) {
        const EventConfig& cfg = kEventConfigs[i];
        // "&&" 且：既是早8点类 又 今天触发了 才记录
        if (cfg.scope == EventScope::DAILY_08 &&
            events_.hasEvent(cfg.type)) {
            names.push_back(cfg.name);   // push_back = 往列表末尾追加一个名字
        }
    }
    return names;   // 把列表交回去
}

// 采矿随机事件：直接把事件系统判定的结果转发出去
void WeatherController::rollMiningEvent(bool& foundChest, bool& caveIn) {
    events_.rollMiningEvent(foundChest, caveIn);
}