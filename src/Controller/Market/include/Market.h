#ifndef MARKET_H
#define MARKET_H

#include <map>
#include <string>
#include <vector>
#include "Object.h"
#include "Shop.h"
#include "Time.h"

// 集市系统：管理商店交易、供需波动与集市日历。
// 为避免依赖尚未实现的 Player / Inventory，交易操作通过
// 金币引用与 Object* 物品参数解耦。
class Market
{
public:
    explicit Market(int dayOfWeek = 1, int dayOfMonth = 1);
    ~Market();

    // ---- 集市日历 ----
    int getDayOfWeek() const;      // 1-7（周一至周日）
    int getDayOfMonth() const;     // 1-30
    bool isProsperousDay() const;  // 是否繁华集（周三）
    bool isFestival() const;       // 是否节日集（每月 1 日）

    // 进入新的一天，根据星期/日期推进日历并刷新价格浮动
    void onNewDay(int week, int day);
    // 从游戏时间对接：由 Time::day 推出星期(1-7)与日期(1-30)再进入新的一天
    void onNewDay(const Time& time);

    // ---- 商店管理 ----
    void registerShop(const Shop& shop);
    std::size_t shopCount() const;
    Shop& getShop(std::size_t index);
    const Shop& getShop(std::size_t index) const;
    Shop* findShop(const std::string& id);
    // 按物品指针查找所属商店，未找到返回 nullptr
    Shop* findShopByItem(Object* item);

    // ---- 供需波动 ----
    // 每天随机生成 2-3 种在售商品价格浮动 ±30%，
    // 并同步更新对应商店物品的当前价格，返回受影响物品数量
    int updateFluctuations();
    float getFluctuation(Object* item) const;   // 默认 1.0

    // ---- 交易 ----
    // 从指定商店购买 item 的 count 份，从 gold 中扣款。
    // 通过 item 传入空指针并将 gold 作为预算传入也可查询价格。
    bool buy(const std::string& shopId, Object* item,
             int count, int& gold);

    // 出售背包物品，将所得金币累加到 gold，返回实际出售获得金额
    int sell(Object* item, int count, int& gold);

    // 报价查询（含浮动与集市加成）
    int getSellPrice(Object* item) const;
    int getBuyPrice(const std::string& shopId, Object* item) const;

private:
    int dayOfWeek;                        // 星期（1-7）
    int dayOfMonth;                       // 日期（1-30）
    std::vector<Shop> shops;              // 商店列表
    std::map<Object*, float> fluctuations;      // 当日价格浮动系数
    float normalSellPrice(Object* item) const;  // 基础售价（物品自身）
    float buyFactor(Object* item) const;        // 购买价浮动系数
};

#endif //MARKET_H
