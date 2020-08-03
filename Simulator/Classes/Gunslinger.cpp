#include "Gunslinger.h"
#include "Simulator/SimulatorBase/AbilityBuff.h"
#include "Simulator/SimulatorBase/ConditionalApplyDebuff.h"
#include "Simulator/SimulatorBase/abilities.h"
#include "detail/Gunslinger.h"
#include "detail/shared.h"
namespace Simulator {

AbilityPtr Gunslinger::getAbilityInternal(AbilityId id) {

    switch (id) {
    case smuggler_flurry_of_bolts: {
        auto info = detail::getDefaultAbilityInfo(id);
        return std::make_shared<Ability>(id, info);
    }
    case gunslinger_quickdraw: {
        auto info = detail::getDefaultAbilityInfo(id);
        return std::make_shared<Ability>(id, info);
    }
    case gunslinger_illegal_mods: {
        auto info = detail::getDefaultAbilityInfo(id);
        StatChanges sc;
        sc.armorPen = 0.15;
        sc.flatForceTechAccuracy = 0.3;
        sc.flatMeleeRangeAccuracy = 0.3;
        auto ass = std::make_unique<RawSheetBuff>("Illegal Mods", AbilityIds{}, sc);
        ass->setDuration(Second{10});
        ass->setId(id);
        auto abl = std::make_shared<Ability>(id, info);
        abl->addOnHitAction(std::make_shared<ConditionalApplyBuff>(std::move(ass), false));
        return abl;
    }
    case gunslinger_smugglers_luck: {
        auto info = detail::getDefaultAbilityInfo(id);
        StatChanges sc;
        sc.flatForceTechCritChance += 1.0;
        sc.flatMeleeRangeCritChance += 1.0;
        auto ass = std::make_unique<RawSheetBuff>("Smuggler's Luck", AbilityIds{dirty_fighting_wounding_shots}, sc);
        ass->setDuration(Second{20});
        ass->setId(gunslinger_smugglers_luck);
        auto abl = std::make_shared<Ability>(id, info);
        abl->addOnHitAction(std::make_shared<ConditionalApplyBuff>(std::move(ass), false));
        return abl;
    }
    case gunslinger_vital_shot: {
        auto info = detail::getDefaultAbilityInfo(id);
        auto dot = std::make_unique<DOT>(id, info);
        auto abl = detail::createDotAbility(std::move(dot));
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(detail::getGenericDebuff(debuff_marked)));
        return abl;
    }
    case gunslinger_hunker_down: {
        auto info = detail::getDefaultAbilityInfo(id);
        auto abl = std::make_shared<Ability>(id, info);
        if (getEstablishedFoothold()) {
            abl->addOnHitAction(std::make_shared<ConditionalApplyBuff>(
                std::make_unique<detail::GunslingerEntrencedOffenseBuff>(), false));
        }
        abl->setCooldown(info.cooldownTime - getEstablishedFoothold() * Second(10) - getLayLowPassive() * Second(15));
        abl->setCooldownIsAffectedByAlacrity(false);
        return abl;
    }
    case gunslinger_speed_shot: {
        auto info = detail::getDefaultAbilityInfo(id);
        info.coefficients[0].multiplier += 0.05; // Steady Shot
        info.coefficients[1].multiplier += 0.05;
        auto abl = std::make_shared<Ability>(id, info);
        return abl;
    }
    }
    return nullptr;
}
std::vector<BuffPtr> Gunslinger::getStaticBuffs() {
    std::vector<BuffPtr> ret;
    return ret;
}

void Gunslinger::loadOptions(const nlohmann::json &j) {
    Class::loadOptions(j);
    load_to_if(j, key_established_foothold, _EstablishedFoothold);
    load_to_if(j, key_exploited_weakness, _ExploitedWeakness);
    load_to_if(j, key_lay_low, _LayLowPassive);
}

} // namespace Simulator
