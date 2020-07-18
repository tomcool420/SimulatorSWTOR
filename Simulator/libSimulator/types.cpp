#include "types.h"
#include "detail/calculations.h"
#include "Player.h"
#include "Target.h"

namespace Simulator {
StatChanges operator+(const StatChanges &a, const StatChanges &b) {
    StatChanges ret = a;
    ret += b;
    return ret;
}

void operator+=(StatChanges &a, const StatChanges &b) {
#define ADD_SC(value) a.value += b.value;
    ADD_SC(masteryMultiplierBonus);
    ADD_SC(masteryBonus);
    
    ADD_SC(criticalRatingBonus);
    ADD_SC(flatMeleeRangeCritChance);
    ADD_SC(flatForceTechCritChance);
    ADD_SC(flatMeleeRangeCriticalMultiplierBonus);
    ADD_SC(flatForceTechCriticalMultiplierBonus);
    
    ADD_SC(powerBonus);
    ADD_SC(powerMultiplier);

    ADD_SC(bonusDamageMultiplier);

    ADD_SC(alacrityRatingBonus);
    ADD_SC(flatAlacrityBonus);

    ADD_SC(armorPen);
    
    ADD_SC(multiplier);
}

FinalStats getFinalStats(const RawStats &rawStats, const StatChanges &statChanges) {
    Mastery m = (rawStats.master + statChanges.masteryBonus) * (1 + statChanges.masteryMultiplierBonus);
    CriticalRating cr = (rawStats.criticalRating + statChanges.criticalRatingBonus);
    double critChance = 0.05 + detail::getCriticalChance(m) + detail::getCriticalChance(cr);
    FinalStats ret;
    ret.forceTechCritChance = critChance + statChanges.flatMeleeRangeCritChance;
    ret.meleeRangeCritChance = critChance + statChanges.flatForceTechCritChance;

    ret.meleeRangeCritMultiplier = 0.5 + detail::getCriticalMultiplier(cr) + statChanges.flatMeleeRangeCriticalMultiplierBonus;
    ret.forceTechCritMultiplier =  0.5 + detail::getCriticalMultiplier(cr) + statChanges.flatForceTechCriticalMultiplierBonus;

    AlacrityRating ar = rawStats.alacrityRating + statChanges.alacrityRatingBonus;
    ret.alacrity = detail::getAlacrity(ar) + statChanges.flatAlacrityBonus;

    Power p = (rawStats.power + statChanges.powerBonus) * (1.0 + statChanges.powerMultiplier);
    double bonusDamage =
        (detail::getBonusDamage(m) + detail::getBonusDamage(p)) * (1.0 + statChanges.bonusDamageMultiplier);
    ret.meleeRangeBonusDamage = bonusDamage;
    double ftBonusDamage =
        detail::getFTBonusDamage(rawStats.forceTechPower) * (1.0 + statChanges.bonusDamageMultiplier);
    ret.forceTechBonusDamage = bonusDamage + ftBonusDamage;
    ret.weaponDamageMH = rawStats.weaponDamageMH;
    ret.weaponDamageOH = rawStats.weaponDamageOH;
    ret.hasOffhand = rawStats.hasOffhand;
    ret.armorPen = rawStats.armorPen + statChanges.armorPen;
    ret.weaponDamageTypeMH = rawStats.weaponDamageTypeMH;
    ret.weaponDamageTypeOH = rawStats.weaponDamageTypeOH;
    ret.multiplier= statChanges.multiplier;
    return ret;
}

FinalStats getFinalStats(const Ability & ability, const Player & player, const Target &target){
    auto && rs = player.getRawStats();
    auto && sc = player.getCurrentPlayerStats(ability, target);
    return getFinalStats(rs, sc);
}

} // namespace Simulator
