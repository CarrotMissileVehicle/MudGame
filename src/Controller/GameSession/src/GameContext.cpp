/**
 * @file GameContext.cpp
 * @brief GameContext 实现：DTO 装配（源自原 main.cpp 的 make_*_view）。
 *
 * 交互辅助（input_hint / refresh_prompt / msg / render_now / opt_int）
 * 见 GameContextInteraction.cpp。
 */
#include "GameContext.h"

mud::view::TimeView GameContext::make_time_view() const
{
    const auto now = time.now();
    mud::view::TimeView t;
    t.year = now.year; t.month = now.month; t.day = now.day;
    t.hour = now.hour; t.minute = now.minute;
    t.time_scale = time.time_scale();
    return t;
}

mud::view::WeatherView GameContext::make_weather_view() const
{
    mud::view::WeatherView w;
    w.weather = weather.weather_name();
    w.can_fish = weather.can_fish();
    w.can_go_outside = weather.can_go_outside();
    w.auto_water = weather.auto_water();
    w.crop_loss_rate = weather.crop_loss_rate();
    w.mining_exp_bonus = weather.mining_exp_bonus();
    w.fishing_penalty = weather.fishing_penalty();
    w.events = weather.today_event_names();
    return w;
}

PlayerStatus GameContext::make_player_status() const
{
    PlayerStatus s;
    s.position = player.GetPosition();
    s.state = player.GetState();
    s.satiety = player.GetSatiety();
    s.maxSatiety = player.GetMaxSatiety();
    s.farmingExp = player.GetFarmingExp();
    s.fishExp = player.GetFishExp();
    s.mineExp = player.GetMineExp();
    s.bagItems = player.GetBag().GetStackedNames();
    return s;
}

mud::view::FarmView GameContext::make_farm_view() const
{
    mud::view::FarmView f;
    for (std::size_t i = 0; i < farming.farmSize(); ++i)
    {
        const auto& fl = farm.getFarmland(i);
        mud::view::PlotView p;
        p.index = i;
        p.occupied = fl.isOccupied();
        if (fl.isOccupied())
        {
            const std::string cname = crop_names.count(fl.getCrop())
                ? crop_names.at(fl.getCrop()) : "未知作物";
            p.crop_name = cname;
            const int growthCycle = fl.getCrop()->getGrowthCycle();
            p.growth_stage = fl.getGrowthStage() > growthCycle
                ? growthCycle : fl.getGrowthStage(); // 防御性钳制
            p.growth_max = growthCycle;
            p.watered = fl.isWatered();
        }
        f.plots.push_back(p);
    }
    return f;
}

mud::view::FishingView GameContext::make_fishing_view() const
{
    mud::view::FishingView f;
    for (const Fish* fish : fish_pool)
        f.pool.push_back({fish_names.at(fish), fish->getProbability()});
    f.can_fish = weather.can_fish();
    return f;
}

mud::view::MarketView GameContext::make_market_view() const
{
    mud::view::MarketView m;
    m.gold = gold;
    m.day_of_week = market.getDayOfWeek();
    m.prosperous = market.isProsperousDay();
    m.festival = market.isFestival();
    for (std::size_t i = 0; i < market.shopCount(); ++i)
    {
        const Shop& shop = market.getShop(i);
        mud::view::ShopView sv;
        sv.id = shop.getId();
        sv.name = shop.getName();
        for (std::size_t j = 0; j < shop.itemCount(); ++j)
        {
            Object* item = shop.getItem(j).getItem();
            if (item == nullptr) continue;
            mud::view::MarketItemView iv;
            iv.name = item->GetName();
            iv.buy = market.getBuyPrice(shop.getId(), item);
            iv.sell = market.getSellPrice(item);
            sv.items.push_back(iv);
        }
        m.shops.push_back(sv);
    }
    return m;
}

mud::view::ToolsView GameContext::make_tools_view() const
{
    mud::view::ToolsView t;
    for (const auto id :
         {mud::tool::ToolId::Hoe, mud::tool::ToolId::Rod, mud::tool::ToolId::Pickaxe})
    {
        mud::view::ToolView tv;
        tv.name = tools.name(id);
        tv.durability = tools.durability(id);
        tv.level = tools.level(id);
        tv.broken = tools.is_broken(id);
        t.tools.push_back(tv);
    }
    return t;
}

mud::view::MiningView GameContext::make_mining_view() const
{
    mud::view::MiningView m;
    m.is_mining = mining.is_mining();
    m.layer = mining.layer_id() ? *mining.layer_id() : 0;
    m.start_time = mining.start_time();
    m.mining_level = 1 + player.GetMineExp() / 100;
    return m;
}

mud::view::BlacksmithView GameContext::make_blacksmith_view() const
{
    mud::view::BlacksmithView v;
    v.gold = gold;
    for (const auto id :
         {mud::tool::ToolId::Hoe, mud::tool::ToolId::Rod, mud::tool::ToolId::Pickaxe})
    {
        mud::view::ToolRepairView tr;
        tr.name = tools.name(id);
        tr.durability = tools.durability(id);
        tr.max_durability = tools.max_durability(id);
        tr.broken = tools.is_broken(id);
        tr.repair_gold = tools.gold_repair_cost(id);
        tr.repair_ore = tools.repair_ore(id);
        tr.repair_ore_needed = tools.repair_ore_count(id);
        tr.repair_ore_held = player.GetBag().CountObject(tools.repair_ore(id));
        v.tools.push_back(tr);
    }
    return v;
}