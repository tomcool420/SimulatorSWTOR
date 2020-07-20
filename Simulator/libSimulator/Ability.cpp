#include "Ability.h"
#include "Target.h"
#include "constants.h"
#include "detail/calculations.h"
#include <random>
#include "utility.h"
namespace Simulator {
DamageRanges calculateDamageRange(const Ability &iAbility, const FinalStats &stats) {
    DamageRanges ret;
    const auto & ability = iAbility.getCoefficients();
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
        ret.push_back({iAbility.getId(), stats.weaponDamageTypeMH, {min, max}, false});
        if (!stats.hasOffhand)
            break;
        min = (stats.weaponDamageOH.first * (1 + ability.AmountModifierPercent)) * (1.0 + stats.multiplier);
        max = (stats.weaponDamageOH.second * (1 + ability.AmountModifierPercent)) * (1.0 + stats.multiplier);
        ret.push_back({iAbility.getId(), stats.weaponDamageTypeOH, {min, max}, true});
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
        ret.push_back({iAbility.getId(), ability.damageType, {min, max}, false});
    }
    }
    return ret;
}
std::random_device rd;
std::mt19937 gen(rd());
double getRandomValue(double min, double max){
    std::uniform_real_distribution<> distrib(min, max);
    return distrib(gen);
}
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
DamageHits getHits(const Ability & ability, const FinalStats & stats, const Target &t){
    auto damageRange = calculateDamageRange(ability, stats);
    auto damageHits = adjustForHitsAndCrits(damageRange, stats, t);
    return adjustForDefensives(damageHits, stats, t);
}

DamageHits adjustForDefensives(const DamageHits &hits, const FinalStats &stats, const Target &t) {
    DamageHits ret(hits);
    Armor armor = t.getArmor() * (1 - std::min(1.0, stats.armorPen));
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


void Ability::onAbilityHitTarget(const DamageHits &hits,const  TargetPtr &source, const TargetPtr &target, const Second &time){
    for(auto && oha : _onHitActions){
        oha->doOnHit(hits, source, target, time);
    }
}
void Ability::addOnHitAction(const OnHitActionPtr & oha){
    CHECK(oha);
    _onHitActions.push_back(oha);
    
}

void applyDamageToTarget(DamageHits hits, Target & source, Target & target,const Second& time){
    DamageHits extraHits;
    DamageHits currentHits = hits;
    DamageHits finalHits;
    while(currentHits.size()){
        for(auto && [aid,buff] : source.getBuffs()){
            auto th = buff->onAbilityHit(currentHits, time, source, target);
            extraHits.insert(extraHits.end(), th.begin(),th.end());
        }
        for(auto && [aid, iDebuffs] : target.getDebuffs()){
            for(auto && [pid, debuff] : iDebuffs){
                auto th = debuff->onAbilityHit(currentHits, time, source, target);
                extraHits.insert(extraHits.end(), th.begin(),th.end());
            }
        }
        finalHits.insert(finalHits.begin(), currentHits.begin(),currentHits.end());
        currentHits = std::move(extraHits);
    }
    target.applyDamageHit(finalHits, target, time);
}
} // namespace Simulator
