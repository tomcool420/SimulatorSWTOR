#include "Ability.h"
#include "Target.h"
#include "constants.h"
#include "detail/calculations.h"
#include "utility.h"
#include <Simulator/SimulatorBase/detail/log.h>
#include <random>

namespace Simulator {
DamageRanges calculateDamageRange(const Ability &iAbility, const AllFinalStats &astats) {
    DamageRanges ret;
    auto &&coefficients = iAbility.getCoefficients();
    CHECK(astats.size() == coefficients.size());
    for (int ii = 0; ii < astats.size(); ++ii) {
        const auto &ability = coefficients[ii];
        const auto &stats = astats[ii];
        switch (ability.damageType) {
        case DamageType::Weapon: {
            auto weapDamage = ability.isOffhandHit ? stats.weaponDamageOH : stats.weaponDamageMH;

            double min = (stats.meleeRangeBonusDamage * ability.coefficient +
                          weapDamage.first * (1.0 + ability.AmountModifierPercent) +
                          ability.StandardHealthPercentMin * StandardHealth) *
                         (1.0 + stats.multiplier + ability.multiplier);
            double max = (stats.meleeRangeBonusDamage * ability.coefficient +
                          weapDamage.second * (1.0 + ability.AmountModifierPercent) +
                          ability.StandardHealthPercentMax * StandardHealth) *
                         (1.0 + stats.multiplier + ability.multiplier);
            ret.push_back({iAbility.getId(), stats.weaponDamageTypeMH, {min, max}, ability.isOffhandHit});
            ret.back().aoe = ability.isAreaOfEffect;
        } break;
        case DamageType::Energy:
        case DamageType::Kinetic:
        case DamageType::Internal:
        case DamageType::Elemental: {
            double min =
                (stats.forceTechBonusDamage * ability.coefficient + ability.StandardHealthPercentMin * StandardHealth) *
                (1.0 + stats.multiplier + ability.multiplier);
            double max =
                (stats.forceTechBonusDamage * ability.coefficient + ability.StandardHealthPercentMax * StandardHealth) *
                (1.0 + stats.multiplier + ability.multiplier);
            ret.push_back({iAbility.getId(), ability.damageType, {min, max}, false});
            ret.back().aoe = ability.isAreaOfEffect;
        }
        }
    }

    return ret;
}
std::random_device rd;
std::mt19937 gen(rd());
double getRandomValue(double min, double max) {
    std::uniform_real_distribution<> distrib(min, max);
    return distrib(gen);
}
DamageHits adjustForHitsAndCrits(const DamageRanges &ranges, const AllFinalStats &astats, const TargetPtr &t) {
    DamageHits ret;
    std::uniform_real_distribution<> distrib(0.0, 1.0);
    CHECK(astats.size() == ranges.size());

    for (int ii = 0; ii < astats.size(); ++ii) {
        auto &&range = ranges[ii];
        auto &&stats = astats[ii];
        DamageHit hit{range.id, range.dt, range.dmg.first + (range.dmg.second - range.dmg.first) * distrib(gen),
                      range.offhand};
        hit.aoe = range.aoe;
        double accuracy = stats.accuracy - t->getDefenseChance() - (hit.offhand ? 0.3 : 0.0);
        if (accuracy < 1.0 && distrib(gen) > accuracy) {
            hit.miss = true;
            hit.dmg = 0.0;
            ret.push_back(hit);
            continue;
        }

        double critChance = 0.0;
        if (hit.dt == DamageType::Weapon) {
            critChance += stats.meleeRangeCritChance;
        } else {
            critChance += stats.forceTechCritChance;
        }

        if (critChance > 1.0 || distrib(gen) < critChance) {
            hit.crit = true;
        }

        if (hit.crit) {
            double critMultiplier = std::max(0.0, critChance - 1.0); // add supercrit to multiplier
            if (hit.dt == DamageType::Weapon) {
                critMultiplier += stats.meleeRangeCritMultiplier;
            } else {
                critMultiplier += stats.forceTechCritMultiplier;
            }
            hit.dmg *= (1 + critMultiplier);
        }
        ret.push_back(hit);
    }
    return ret;
}
DamageHits getHits(const Ability &ability, const AllFinalStats &stats, const TargetPtr &t) {
    auto damageRange = calculateDamageRange(ability, stats);
    auto damageHits = adjustForHitsAndCrits(damageRange, stats, t);
    return adjustForDefensives(damageHits, stats, t);
}

DamageHits adjustForDefensives(const DamageHits &hits, const AllFinalStats &stats, const TargetPtr &t) {
    DamageHits ret(hits);
    CHECK(hits.size() == stats.size());
    for (int ii = 0; ii < stats.size(); ++ii) {
        Armor armor = t->getArmor() * (1 - std::min(1.0, stats[ii].armorPen)) * (1.0 - 0.2 * stats[ii].armorDebuff);
        auto dr = detail::getDamageReduction(armor);
        auto &hit = ret[ii];
        if (hit.miss)
            continue;
        if (hit.dt == DamageType::Internal) {
            hit.dmg *= (1.0 - t->getInternalDR());
        } else if (hit.dt == DamageType::Elemental) {
            hit.dmg *= (1.0 - t->getElementalDR());
        } else {
            hit.dmg *= (1.0 - dr);
        }
    }
    return ret;
}

void Ability::onAbilityHitTarget(const DamageHits &hits, const TargetPtr &source, const TargetPtr &target,
                                 const Second &time) {
    for (auto &&oha : _onHitActions) {
        oha->doOnHit(hits, source, target, time);
    }
}
void Ability::addOnHitAction(const OnHitActionPtr &oha) {
    CHECK(oha);
    _onHitActions.push_back(oha);
}
void Ability::onAbilityEnd(const TargetPtr &source, const TargetPtr &target, const Second &time) {
    for (auto &&oha : _onEndActions) {
        oha(source, target, time);
    }
}
Second Ability::getTravelTime() const {
    if (_info.travelTime > Second(0.01))
        return _info.travelTime;
    if (_info.travelSpeed > 0.001)
        return Second(0.15);
    return Second(0.0);
}

void applyDamageToTarget(DamageHits hits, const TargetPtr &source, const TargetPtr &target, const Second &time) {
    DamageHits currentHits = hits;
    while (currentHits.size()) {
        DamageHits extraHits;
        for (auto &&[aid, buff] : source->getBuffs()) {
            auto th = buff->onAbilityHit(currentHits, time, source, target);
            extraHits.insert(extraHits.end(), th.begin(), th.end());
        }
        for (auto &&[aid, iDebuffs] : target->getDebuffs()) {
            for (auto &&[pid, debuff] : iDebuffs) {
                auto th = debuff->onAbilityHit(currentHits, time, source, target);
                extraHits.insert(extraHits.end(), th.begin(), th.end());
            }
        }
        target->applyDamageHit(currentHits, target, time);
        currentHits = std::move(extraHits);
    }
}
} // namespace Simulator
