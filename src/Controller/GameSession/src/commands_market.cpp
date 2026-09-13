/**
 * @file commands_market.cpp
 * @brief 集市指令注册（market.status / market.buy / market.sell）。
 */
#include "GameCommands.h"

#include "GameContext.h"
#include "connector.h"

#include <algorithm>
#include <string>

void GameCommands::register_market(Connector& connector, GameContext& ctx)
{
    connector.bind("market.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        ctx.view.render_market(ctx.make_market_view());
        return HandlerResult::Ok;
    });

    connector.bind("market.buy", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtTown) {
            ctx.msg("你不在城镇。");
            return HandlerResult::Failed;
        }
        const std::string shop_id = cmd.options.count("shop") ? cmd.options.at("shop") : "";
        const std::string item_name = cmd.options.count("item") ? cmd.options.at("item") : "";
        const auto count = static_cast<std::size_t>(ctx.opt_int(cmd, "count", 1));
        Shop* shop = ctx.market.findShop(shop_id);
        if (shop == nullptr) {
            ctx.msg("没有这家商店：" + shop_id);
            return HandlerResult::BadArgument;
        }
        Object* item = nullptr;
        const auto all_digits = [](const std::string& s) {
            if (s.empty()) return false;
            for (char c : s) if (c < '0' || c > '9') return false;
            return true;
        };
        if (all_digits(item_name)) {
            const std::size_t idx = static_cast<std::size_t>(ctx.opt_int(cmd, "item", 0));
            if (idx < shop->itemCount()) item = shop->getItem(idx).getItem();
        } else {
            for (std::size_t i = 0; i < shop->itemCount(); ++i) {
                Object* cand = shop->getItem(i).getItem();
                if (cand != nullptr && cand->GetName() == item_name) { item = cand; break; }
            }
        }
        if (item == nullptr) {
            ctx.msg("商店没有这种商品：" + item_name);
            return HandlerResult::BadArgument;
        }
        const int price = ctx.market.getBuyPrice(shop_id, item);
        if (!ctx.market.buy(shop_id, item, static_cast<int>(count), ctx.gold)) {
            ctx.msg("购买失败（金币不足或商品缺货）。");
            return HandlerResult::Failed;
        }
        for (std::size_t i = 0; i < count; ++i)
            ctx.player.GetBag().AddObject(new Object(item->GetName(), item->GetDescription(),
                item->GetHealth(), item->GetSellingPrice(), item->GetBuyingPrice()));
        ctx.msg("购入 " + item->GetName() + " x" + std::to_string(count)
            + "（花费 " + std::to_string(price * count) + "）。");
        return HandlerResult::Ok;
    });

    connector.bind("market.sell", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtTown) {
            ctx.msg("你不在城镇。");
            return HandlerResult::Failed;
        }
        const std::string raw_item = cmd.options.count("item") ? cmd.options.at("item") : "";
        const auto count = static_cast<std::size_t>(ctx.opt_int(cmd, "count", 1));
        const auto all_digits = [](const std::string& s) {
            if (s.empty()) return false;
            for (char c : s) if (c < '0' || c > '9') return false;
            return true;
        };
        // 去重后的同名词（每个堆叠一项）
        const auto& all_names = ctx.player.GetBag().GetAllObjectName();
        std::string item_name;
        if (all_digits(raw_item)) {
            const std::size_t idx = static_cast<std::size_t>(ctx.opt_int(cmd, "item", 0));
            if (idx >= all_names.size()) {
                ctx.msg("背包里没有序号 " + std::to_string(idx) + " 的物品。");
                return HandlerResult::BadArgument;
            }
            item_name = all_names[idx];
        } else {
            item_name = raw_item;
            bool found = false;
            for (const auto& n : all_names) if (n == item_name) { found = true; break; }
            if (!found) {
                ctx.msg("背包里没有：" + item_name);
                return HandlerResult::BadArgument;
            }
        }
        // 堆叠总数量
        const int have = ctx.player.GetBag().CountObject(item_name);
        const auto sell_count = static_cast<std::size_t>(std::min<long long>(
            static_cast<long long>(count), static_cast<long long>(have)));
        if (sell_count == 0) {
            ctx.msg("背包里没有：" + item_name);
            return HandlerResult::BadArgument;
        }
        Object* item = nullptr;
        for (auto* obj : ctx.player.GetBag().GetObjects()) {
            if (obj->GetName() == item_name) { item = obj; break; }
        }
        if (item == nullptr) {
            ctx.msg("背包里没有：" + item_name);
            return HandlerResult::BadArgument;
        }
        const int gained = ctx.market.sell(item, static_cast<int>(sell_count), ctx.gold);
        ctx.player.GetBag().RemoveObject(item_name, static_cast<int>(sell_count));
        ctx.msg("出售" + item_name + " x" + std::to_string(sell_count) + "，获得金币 "
            + std::to_string(gained) + "。");
        return HandlerResult::Ok;
    });
}