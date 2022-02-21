#include "Sharpshooter.h"
#include "Simulator/SimulatorBase/AbilityBuff.h"
#include "Simulator/SimulatorBase/ConditionalApplyDebuff.h"
#include "Simulator/SimulatorBase/StatBuff.h"
#include "Simulator/SimulatorBase/abilities.h"
#include "detail/Gunslinger.h"
#include "detail/shared.h"

namespace Simulator {
namespace detail {

class LuckyDraw : public Buff {
    void OnAbilityHitTarget(const Ability &, const DamageHits &hits, const TargetPtr &player,
                            const Second &time) const override {
        auto cd = player->getAbilityCooldownEnd(gunslinger_smugglers_luck);
        if (!cd)
            return; // smuggler's luck is not on CD
        for (auto &&hit : hits) {
            if (hit.id == sharpshooter_penetrating_blasts && hit.offhand == false) {
                if (*cd - time < Second(2))
                    player->finishCooldown(gunslinger_smugglers_luck, time);
                else {
                    player->setAbilityCooldown(gunslinger_smugglers_luck, *cd - Second(2));
                }
                SIM_INFO("Lucky Draw, Reducing CD of Smuggler's luck");
            }
        }
    }
};

class LuckyThoughts : public Buff {
    void OnAbilityHitTarget(const Ability &, const DamageHits &hits, const TargetPtr &player,
                            const Second &time) const override {
        auto cd = player->getAbilityCooldownEnd(gunslinger_smugglers_luck);
        if (!cd)
            return; // smuggler's luck is not on CD
        for (auto &&hit : hits) {
            if (hit.dt == DamageType::Weapon && hit.crit == true) {
                if (*cd - time < Second(2))
                    player->finishCooldown(gunslinger_smugglers_luck, time);
                else {
                    player->setAbilityCooldown(gunslinger_smugglers_luck, *cd - Second(2));
                }
                SIM_INFO("Lucky Thoughts, Reducing CD of Smuggler's luck");
            }
        }
    }
};

BuffPtrs getTalentPassives(const std::array<uint8_t, 8> &talents) {
    BuffPtrs ret;

    if (talents[0] == 0) {
        StatChanges sc;
        sc.flatMeleeRangeCriticalMultiplierBonus = 0.2;
        ret.push_back(std::make_unique<AbilityStatBuff>(sc, AbilityIds{sharpshooter_aimed_shot}));
    } else if (talents[0] == 2) {
        // TODO
    }

    if (talents[1] == 0) {
        StatChanges sc;
        sc.flatMeleeRangeCriticalMultiplierBonus = 0.2;
        ret.push_back(std::make_unique<AbilityStatBuff>(sc, AbilityIds{gunslinger_charged_burst}));
    }

    if (talents[2] == 0) {
        StatChanges sc;
        sc.armorPen = 0.2;
        ret.push_back(std::make_unique<AbilityStatBuff>(sc, AbilityIds{sharpshooter_penetrating_blasts}));
    } else if (talents[2] == 1) {
        StatChanges sc;
        sc.flatMeleeRangeCritChance = 0.2;
        sc.flatMeleeRangeCriticalMultiplierBonus = 0.2;
        ret.push_back(std::make_unique<AbilityStatBuff>(sc, AbilityIds{sharpshooter_penetrating_blasts}));
    } else if (talents[2] == 2) {
        ret.push_back(std::make_unique<LuckyDraw>());
    }

    if (talents[3] == 0) {
        ret.push_back(std::make_unique<LuckyThoughts>());
    }
    return ret;
}

BuffPtrs getClassBuffs() {
    BuffPtrs ret;
    // AimHigh
    {
        StatChanges sc;
        sc.flatMeleeRangeCritChance = 0.15;
        AbilityIds abls{sharpshooter_penetrating_blasts, gunslinger_quickdraw};
        ret.push_back(std::make_unique<AbilityStatBuff>(sc, abls));
    }
    // Deadeye
    {
        StatChanges sc;
        sc.flatMeleeRangeCriticalMultiplierBonus = 0.10;
        AbilityIds abls{sharpshooter_aimed_shot, sharpshooter_penetrating_blasts, sharpshooter_trickshot,
                        gunslinger_charged_burst, gunslinger_quickdraw};
        ret.push_back(std::make_unique<AbilityStatBuff>(sc, abls));
    }
    // Steady Shots
    {
        StatChanges sc;
        sc.multiplier += 0.05;
        AbilityIds abls{sharpshooter_penetrating_blasts, gunslinger_charged_burst};
        ret.push_back(std::make_unique<AbilityStatBuff>(sc, abls));
    }
    // TODO Accurized Blasters, sweeping gunfire
    return ret;
}
} // namespace detail

BuffPtrs Sharpshooter::getStaticBuffs() {
    auto ret = Gunslinger::getStaticBuffs();
    auto cb = detail::getClassBuffs();
    ret.insert(ret.end(), std::make_move_iterator(cb.begin()), std::make_move_iterator(cb.end()));
    return ret;
}

AbilityPtr Sharpshooter::getAbilityInternal(AbilityId id) {
    switch (id) {
    case sharpshooter_penetrating_blasts: {
        auto info = detail::getDefaultAbilityInfo(sharpshooter_penetrating_blasts);
        auto abl = std::make_shared<Ability>(id, std::move(info));
        return abl;
    }
    case sharpshooter_aimed_shot: {
        auto info = detail::getDefaultAbilityInfo(sharpshooter_aimed_shot);
        auto abl = std::make_shared<Ability>(id, std::move(info));
        return abl;
    }
    default:
        return Gunslinger::getAbilityInternal(id);
    }
}
nlohmann::json Sharpshooter::serialize() {
    auto j = Gunslinger::serialize();
    j[key_class] = key_class_sharpshooter;
    return j;
}
} // namespace Simulator