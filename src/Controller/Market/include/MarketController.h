#ifndef MUDGAME_MARKET_CONTROLLER_H
#define MUDGAME_MARKET_CONTROLLER_H

#include <string>
#include "Market.h"
#include "Bag.h"

// 集市交易控制器：承载 market.buy / market.sell 命令的完整业务流程
// （商店/物品解析、扣款入包、数量钳制、售价守卫），
// 使 main.cpp 处理器退化为参数提取 + 结果渲染。
class MarketController
{
public:
    struct BuyResult
    {
        enum class Status
        {
            Ok,           // 成功：金币已扣、物品已入包
            NoSuchShop,   // 商店不存在
            NoSuchItem,   // 商品不存在（名字或序号未命中）
            BuyRejected   // Market::buy 拒绝（金币不足/非正数量/价格无效）
        };
        Status status = Status::Ok;
        std::string item_name;  // Ok 时有效
        int bought = 0;          // Ok 时有效
        long long spent = 0;     // Ok 时有效（64 位总额，DEF-109）
    };

    struct SellResult
    {
        enum class Status
        {
            Ok,          // 成功：物品已移除、金币已加
            NoSuchItem,  // 物品不存在或可售数量为 0
            ZeroPrice    // 售价经浮动截断为 0：不移除不加钱（DEF-106）
        };
        Status status = Status::Ok;
        std::string item_name;  // Ok 时有效
        int sold = 0;           // Ok 时有效
        long long gained = 0;   // Ok 时有效
    };

    explicit MarketController(Market& market, Bag& bag);

    /**
     * @brief 从商店购买：item_ref 可为商品名或商店内序号（纯数字串）。
     *
     * 成功时扣 gold、按 count 入包拷贝；任何失败路径均不改变 gold 与背包。
     */
    BuyResult buy(const std::string& shop_id, const std::string& item_ref,
                  int count, long long& gold);

    /**
     * @brief 向集市出售：item_ref 可为背包物品名或背包序号（纯数字串），
     *        count 钳制到实际持有量。
     *
     * 成功时移除物品、累加 gold；售价为 0 时不移除（DEF-106）。
     */
    SellResult sell(const std::string& item_ref, int count, long long& gold);

private:
    Market& market_;
    Bag& bag_;
};

#endif //MUDGAME_MARKET_CONTROLLER_H
