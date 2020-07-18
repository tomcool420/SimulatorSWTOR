#include "Ability.h"
#include "Target.h"
#include "constants.h"
#include "detail/calculations.h"
#include <random>

namespace Simulator {
DamageRanges calculateDamageRange(const Ability &ability, const FinalStats &stats) {
    DamageRanges ret;

    switch (ability.damageType) {
    case DamageType::Weapon: {
        double min = (stats.meleeRangeBonusDamage * ability.coefficient +
                      stats.weaponDamageMH.first * (1.0 + ability.AmountModifierPercent) +
                      ability.StandardHealthPercentMin * StandardHealth) *
                     (1.0 + stats.multiplier);
        double max = (stats.meleeRangeBonusDamage * ability.coefficient +
                      stats.weaponDamageMH.second * (1.0 + ability.AmountModifierPercent) +
                      ability.StandardHealthPercentMax * StandardHealth) *
                     (1.0 + stats.multiplier);
        ret.push_back({ability.id, stats.weaponDamageTypeMH, {min, max}, false});
        if (!stats.hasOffhand)
            break;
        min = (stats.weaponDamageOH.first * (1 + ability.AmountModifierPercent)) * (1.0 + stats.multiplier);
        max = (stats.weaponDamageOH.second * (1 + ability.AmountModifierPercent)) * (1.0 + stats.multiplier);
        ret.push_back({ability.id, stats.weaponDamageTypeOH, {min, max}, true});
    } break;
    case DamageType::Energy:
    case DamageType::Kinetic:
    case DamageType::Internal:
    case DamageType::Elemental: {
        double min =
            (stats.forceTechBonusDamage * ability.coefficient + ability.StandardHealthPercentMin * StandardHealth) *
            (1.0 + stats.multiplier);
        double max =
            (stats.forceTechBonusDamage * ability.coefficient + ability.StandardHealthPercentMax * StandardHealth) *
            (1.0 + stats.multiplier);
        ret.push_back({ability.id, ability.damageType, {min, max}, false});
    }
    }
    return ret;
}
std::random_device rd;
std::mt19937 gen(rd());

DamageHits adjustForHitsAndCrits(const DamageRanges &ranges, const FinalStats &stats, const Target &t) {
    DamageHits ret;
    std::uniform_real_distribution<> distrib(0.0, 1.0);

    for (auto &&range : ranges) {
        DamageHit hit{range.id, range.dt, range.dmg.first + (range.dmg.second - range.dmg.first) * distrib(gen),
                      range.offhand};
        double accuracy = stats.accuracy - t.getDefenseChance() - (hit.offhand ? 0.3 : 0.0);
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

DamageHits adjustForDefensives(const DamageHits &hits, const FinalStats &stats, const Target &t) {
    DamageHits ret(hits);
    double armor = t.getArmor() * (1 - std::min(1.0, stats.armorPen));
    auto dr = detail::getDamageReduction(armor);
    for (auto &&hit : ret) {
        if (hit.miss)
            continue;
        if (hit.dt == DamageType::Internal) {
            hit.dmg *= (1.0 - t.getInternalDR());
        } else if (hit.dt == DamageType::Elemental) {
            hit.dmg *= (1.0 - t.getElementalDR());
        } else {
            hit.dmg *= (1.0 - dr);
        }
    }
    return ret;
}

} // namespace Simulator
