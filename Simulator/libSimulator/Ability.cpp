#include "Ability.h"
#include "constants.h"
#include <random>
#include "Target.h"
#include "detail/calculations.h"

namespace Simulator {
DamageHits calculateDamageRange(const Ability &ability, const FinalStats &stats) {
    DamageHits ret;
    switch (ability.damageType) {
    case DamageType::Weapon: {
        double min = (stats.meleeRangeBonusDamage * ability.coefficient +
                     stats.weaponDamageMH.first * (1.0 + ability.AmountModifierPercent) +
                     ability.StandardHealthPercentMin * StandardHealth)*(1.0+stats.multiplier);
        double max = (stats.meleeRangeBonusDamage * ability.coefficient +
                     stats.weaponDamageMH.second * (1.0 + ability.AmountModifierPercent) +
                     ability.StandardHealthPercentMax * StandardHealth)*(1.0+stats.multiplier);
        ret.push_back({ability.id, ability.damageType, {min, max},false});
        if (!stats.hasOffhand)
            break;
        min = (stats.weaponDamageOH.first * (1 + ability.AmountModifierPercent))*(1.0+stats.multiplier);
        max = (stats.weaponDamageOH.second * (1 + ability.AmountModifierPercent))*(1.0+stats.multiplier);
        ret.push_back({ability.id, ability.damageType, {min, max},true});
    } break;
    case DamageType::Energy:
    case DamageType::Kinetic:
    case DamageType::Internal:
    case DamageType::Elemental: {
        double min =
        (stats.forceTechBonusDamage * ability.coefficient + ability.StandardHealthPercentMin * StandardHealth)*(1.0+stats.multiplier);
        double max =
            (stats.forceTechBonusDamage * ability.coefficient + ability.StandardHealthPercentMax * StandardHealth)*(1.0+stats.multiplier);
        ret.push_back({ability.id, ability.damageType, {min, max},false});
    }
    }
    return ret;
}
std::random_device rd;
std::mt19937 gen(rd());

DamageHits adjustForHitsAndCrits(const DamageHits & hits, const FinalStats &stats, const Target &t){
    DamageHits ret(hits);
    std::uniform_real_distribution<> distrib(0.0, 1.0);

    for(auto && hit : ret){
        double accuracy = stats.accuracy-t.getDefenseChance()- (hit.offhand?0.3:0.0);
        if(accuracy<1.0 && distrib(gen)>accuracy){
            hit.miss=true;
            hit.dmg.first=0.0;
            hit.dmg.second=0.0;
            continue;
        }
        
        double critChance = 0.0;
        if(hit.dt==DamageType::Weapon){
            critChance+=stats.meleeRangeCritChance;
        }else{
            critChance+=stats.forceTechCritChance;
        }
        
        if(critChance>1.0 || distrib(gen)<critChance){
            hit.crit=true;
        }
        
        if(hit.crit){
            double critMultiplier = std::max(0.0,critChance-1.0); // add supercrit to multiplier
            if(hit.dt==DamageType::Weapon){
                critMultiplier+=stats.meleeRangeCritMultiplier;
            }else{
                critMultiplier+=stats.forceTechCritMultiplier;
            }
            hit.dmg.first*=(1+critMultiplier);
            hit.dmg.second*=(1+critMultiplier);
        }
        
    }
    return ret;
}

DamageHits adjustForDefensives(const DamageHits & hits, const FinalStats &stats, const Target &t){
    DamageHits ret(hits);
    double armor = t.getArmor()*(1-std::min(1.0,stats.armorPen));
    auto dr = detail::getDamageReduction(armor);
        for(auto && hit : ret){
            if(hit.miss)
                continue;
            if(hit.dt==DamageType::Internal){
                hit.dmg.first*=(1.0-t.getInternalDR());
                hit.dmg.second*=(1.0-t.getInternalDR());
            }else if(hit.dt==DamageType::Elemental){
                hit.dmg.first*=(1.0-t.getElementalDR());
                hit.dmg.second*=(1.0-t.getElementalDR());
            }else{
                hit.dmg.first*=(1.0-dr);
                hit.dmg.second*=(1.0-dr);
            }
        }
    return ret;
}

} // namespace Simulator
