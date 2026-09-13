/**
 * @file commands_farm.cpp
 * @brief 农田指令注册（farm.status / sow / water / fertilize / harvest）。
 */
#include "GameCommands.h"

#include "GameContext.h"
#include "connector.h"

#include <string>

void GameCommands::register_farm(Connector& connector, GameContext& ctx)
{
    connector.bind("farm.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        ctx.view.render_farm(ctx.make_farm_view());
        return HandlerResult::Ok;
    });

    const auto valid_plot = [&](std::size_t idx) {
        return idx < ctx.farming.farmSize();
    };

    connector.bind("farm.sow", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtFarmland) {
            ctx.msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(ctx.opt_int(cmd, "plot", 0));
        const std::string crop_key = cmd.options.count("crop") ? cmd.options.at("crop") : "";
        const auto it = ctx.seeds.find(crop_key);
        if (it == ctx.seeds.end()) {
            ctx.msg("没有这种作物：" + crop_key);
            return HandlerResult::BadArgument;
        }
        const Crop* crop = it->second;
        if (crop->getUnlockLevel() > 1 + ctx.player.GetFarmingExp() / 100) {
            ctx.msg("作物未解锁（需种植经验 ≥ " + std::to_string(crop->getUnlockLevel() * 100) + "）。");
            return HandlerResult::Failed;
        }
        if (!valid_plot(idx) || !ctx.farming.sow(idx, it->second)) {
            ctx.msg("播种失败（地块占用或索引越界）。");
            return HandlerResult::Failed;
        }
        ctx.player.SetState(StateCode::Seeding);
        ctx.msg("已播种 " + ctx.crop_names.at(crop) + "。");
        ctx.player.SetState(StateCode::Waiting); // 播种为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.water", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtFarmland) {
            ctx.msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(ctx.opt_int(cmd, "plot", 0));
        if (!valid_plot(idx) || !ctx.farming.water(idx)) {
            ctx.msg("浇水失败（地块无作物或索引越界）。");
            return HandlerResult::Failed;
        }
        ctx.player.SetState(StateCode::Watering);
        ctx.weather.mark_watered(); // 手动浇水计入"今日已浇水"
        ctx.msg("已浇水。");
        ctx.player.SetState(StateCode::Waiting); // 浇水为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.fertilize", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtFarmland) {
            ctx.msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(ctx.opt_int(cmd, "plot", 0));
        const std::string type = cmd.options.count("type") ? cmd.options.at("type") : "";
        Fertilizer* fert = nullptr;
        if (type == "normal") fert = &ctx.normal_fert;
        else if (type == "advanced") fert = &ctx.advanced_fert;
        if (fert == nullptr) {
            ctx.msg("未知肥料类型：" + type + "（normal/advanced）");
            return HandlerResult::BadArgument;
        }
        if (!valid_plot(idx) || !ctx.farming.fertilize(idx, fert)) {
            ctx.msg("施肥失败（地块无作物或索引越界）。");
            return HandlerResult::Failed;
        }
        ctx.player.SetState(StateCode::Fertilizing);
        ctx.msg("施肥完成。");
        ctx.player.SetState(StateCode::Waiting); // 施肥为一次性动作，完成后复位
        return HandlerResult::Ok;
    });

    connector.bind("farm.harvest", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtFarmland) {
            ctx.msg("你不在农田。");
            return HandlerResult::Failed;
        }
        const std::size_t idx = static_cast<std::size_t>(ctx.opt_int(cmd, "plot", 0));
        if (!valid_plot(idx)) {
            ctx.msg("地块索引越界。");
            return HandlerResult::BadArgument;
        }
        auto& fl = ctx.farm.getFarmland(idx);
        const Crop* crop = fl.getCrop();
        const int yield = ctx.farming.harvest(idx);
        if (yield <= 0) {
            ctx.msg("尚无成熟作物。");
            return HandlerResult::Failed;
        }
        const std::string cname = ctx.crop_names.at(crop);
        const int sell = crop->GetSellingPrice();
        const int buy = crop->GetBuyingPrice();
        for (int i = 0; i < yield; ++i)
            ctx.player.GetBag().AddObject(new Object(cname + "（收获）", "农田收获", 0, sell, buy));
        ctx.player.SetFarmingExp(ctx.player.GetFarmingExp() + crop->getFarmExp());
        ctx.msg("收获 " + cname + " x" + std::to_string(yield)
            + "（+" + std::to_string(crop->getFarmExp()) + " 种植经验）。");
        return HandlerResult::Ok;
    });
}