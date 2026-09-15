/**
 * @file commands_market.cpp
 * @brief 集市指令注册（market.status / market.buy / market.sell）。
 *
 * 交易流程统一委托 MarketController（含 DEF-106 零价守卫 / DEF-109 64 位
 * 金额 / 数量钳制 / 跨堆叠扣减），命令层只做参数提取与结果渲染，
 * 避免重复实现与控制器业务漂移。
 */
#include "GameCommands.h"

#include "GameContext.h"
#include "MarketController.h"
#include "connector.h"

#include <string>

void GameCommands::register_market(Connector& connector, GameContext& ctx)
{
    connector.bind("market.status", [ctx](const mud::cmd::Command&, const HandlerContext&) {
        ctx.view.render_market(ctx.make_market_view());
        return HandlerResult::Ok;
    });

    connector.bind("market.buy", [ctx](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtTown) {
            ctx.msg("你不在城镇。");
            return HandlerResult::Failed;
        }
        const std::string shop_id = cmd.options.count("shop") ? cmd.options.at("shop") : "";
        const std::string item_ref = cmd.options.count("item") ? cmd.options.at("item") : "";
        const int count = static_cast<int>(ctx.opt_int(cmd, "count", 1));

        MarketController mc(ctx.market, ctx.player.GetBag());
        const auto result = mc.buy(shop_id, item_ref, count, ctx.gold);
        switch (result.status)
        {
        case MarketController::BuyResult::Status::Ok:
            ctx.msg("购入 " + result.item_name + " x" + std::to_string(result.bought)
                + "（花费 " + std::to_string(result.spent) + "）。");
            return HandlerResult::Ok;
        case MarketController::BuyResult::Status::NoSuchShop:
            ctx.msg("没有这家商店：" + shop_id);
            return HandlerResult::BadArgument;
        case MarketController::BuyResult::Status::NoSuchItem:
            ctx.msg("商店没有这种商品：" + item_ref);
            return HandlerResult::BadArgument;
        default: // BuyRejected
            ctx.msg("购买失败（金币不足或商品缺货）。");
            return HandlerResult::Failed;
        }
    });

    connector.bind("market.sell", [ctx](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtTown) {
            ctx.msg("你不在城镇。");
            return HandlerResult::Failed;
        }
        const std::string item_ref = cmd.options.count("item") ? cmd.options.at("item") : "";
        const int count = static_cast<int>(ctx.opt_int(cmd, "count", 1));

        MarketController mc(ctx.market, ctx.player.GetBag());
        const auto result = mc.sell(item_ref, count, ctx.gold);
        switch (result.status)
        {
        case MarketController::SellResult::Status::Ok:
            ctx.msg("出售" + result.item_name + " x" + std::to_string(result.sold)
                + "，获得金币 " + std::to_string(result.gained) + "。");
            return HandlerResult::Ok;
        case MarketController::SellResult::Status::ZeroPrice:
            ctx.msg("该物品当前售价为 0，无法出售。");
            return HandlerResult::Failed;
        default: // NoSuchItem
            ctx.msg("背包里没有：" + item_ref);
            return HandlerResult::BadArgument;
        }
    });
}
