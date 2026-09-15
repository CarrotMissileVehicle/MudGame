#include "../include/MarketController.h"

#include <memory>

namespace
{

// 纯数字串判定：决定 item_ref 按序号还是按名字解析
bool all_digits(const std::string& s)
{
    if (s.empty()) return false;
    for (char c : s)
        if (c < '0' || c > '9') return false;
    return true;
}

// 数字串转非负索引，溢出/非法返回 huge（越界判定统一走找不到路径）
std::size_t to_index(const std::string& s)
{
    try { return static_cast<std::size_t>(std::stoull(s)); }
    catch (...) { return static_cast<std::size_t>(-1); }
}

} // namespace

MarketController::MarketController(Market& market, Bag& bag)
        : market_(market), bag_(bag) {}

MarketController::BuyResult
MarketController::buy(const std::string& shop_id, const std::string& item_ref,
                       int count, long long& gold)
{
    BuyResult result;

    Shop* shop = market_.findShop(shop_id);
    if (shop == nullptr)
    {
        result.status = BuyResult::Status::NoSuchShop;
        return result;
    }

    // 商品解析：纯数字 → 商店内序号；否则 → 商品名
    Object* item = nullptr;
    if (all_digits(item_ref))
    {
        const std::size_t idx = to_index(item_ref);
        if (idx < shop->itemCount()) item = shop->getItem(idx).getItem();
    }
    else
    {
        for (std::size_t i = 0; i < shop->itemCount(); ++i)
        {
            Object* cand = shop->getItem(i).getItem();
            if (cand != nullptr && cand->GetName() == item_ref)
            {
                item = cand;
                break;
            }
        }
    }
    if (item == nullptr)
    {
        result.status = BuyResult::Status::NoSuchItem;
        return result;
    }

    const int price = market_.getBuyPrice(shop_id, item);
    if (!market_.buy(shop_id, item, count, gold))
    {
        // 金币不足/非正数量/价格无效：gold 与背包均未改变
        result.status = BuyResult::Status::BuyRejected;
        return result;
    }

    // 入包等量拷贝（H10：商品实际交付入包，同名自动合并为同一堆叠，
    // 数量与逐件入包一致；unique_ptr 持有至 AddObject 接管，异常安全）
    auto obj = std::make_unique<Object>(item->GetName(), item->GetDescription(),
                                        item->GetHealth(), item->GetSellingPrice(),
                                        item->GetBuyingPrice());
    obj->SetQuantity(count);
    bag_.AddObject(obj.release());

    result.status = BuyResult::Status::Ok;
    result.item_name = item->GetName();
    result.bought = count;
    // 64 位总额（DEF-109：大价格 x 大数量不回绕）
    result.spent = static_cast<long long>(price) * static_cast<long long>(count);
    return result;
}

MarketController::SellResult
MarketController::sell(const std::string& item_ref, int count, long long& gold)
{
    SellResult result;

    // 物品名解析：纯数字 → 背包序号（去重后的同名词列表）；否则 → 名字
    const std::vector<std::string>& all_names = bag_.GetAllObjectName();
    std::string item_name;
    if (all_digits(item_ref))
    {
        const std::size_t idx = to_index(item_ref);
        if (idx >= all_names.size())
        {
            result.status = SellResult::Status::NoSuchItem;
            return result;
        }
        item_name = all_names[idx];
    }
    else
    {
        item_name = item_ref;
        bool found = false;
        for (const auto& n : all_names)
            if (n == item_name)
            {
                found = true;
                break;
            }
        if (!found)
        {
            result.status = SellResult::Status::NoSuchItem;
            return result;
        }
    }

    // 可售数量 = min(请求数, 持有量)；为 0 视为无可售物品
    const int have = bag_.CountObject(item_name);
    const int sell_count = count < have ? count : have;
    if (sell_count <= 0)
    {
        result.status = SellResult::Status::NoSuchItem;
        return result;
    }

    Object* item = nullptr;
    for (const auto& obj : bag_.GetObjects())
    {
        if (obj->GetName() == item_name)
        {
            item = obj.get();
            break;
        }
    }
    if (item == nullptr)
    {
        result.status = SellResult::Status::NoSuchItem;
        return result;
    }

    const long long gained = market_.sell(item, sell_count, gold);
    if (gained <= 0)
    {
        // DEF-106：售价经浮动截断为 0 时不移除物品（否则白送）
        result.item_name = item_name;
        result.status = SellResult::Status::ZeroPrice;
        return result;
    }

    // H9/H11：按实际移除量入账。同名多堆叠（读档 AddUnique 还原）时
    // RemoveObject 跨堆叠累计扣减，实扣不足则回退多记的金币，杜绝刷钱漏洞
    const int removed = bag_.RemoveObject(item_name, sell_count);
    if (removed != sell_count)
    {
        const long long unit = gained / sell_count;   // 单价（截断后整数）
        const long long refund = unit * (sell_count - removed);
        gold -= refund;
        result.sold = removed;
        result.gained = gained - refund;
    }
    else
    {
        result.sold = sell_count;
        result.gained = gained;
    }
    result.status = SellResult::Status::Ok;
    result.item_name = item_name;
    return result;
}
