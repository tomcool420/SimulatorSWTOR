#include "types.h"
#include "detail/calculations.h"

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
    ADD_SC(alacrityRatingBonus);
    ADD_SC(bonusDamageMultiplier);
    ADD_SC(flatAlacrityBonus);
    ADD_SC(flatCriticalBonus);
    ADD_SC(flatCriticalMultiplierBonus);
    ADD_SC(powerBonus);
    ADD_SC(powerMultiplier);
    ADD_SC(armorPen);
}

FinalStats getFinalStats(const RawStats &rawStats, const StatChanges &statChanges) {
    Mastery m = (rawStats.master + statChanges.masteryBonus) * (1 + statChanges.masteryMultiplierBonus);
    CriticalRating cr = (rawStats.criticalRating + statChanges.criticalRatingBonus);
    double critChance =
        0.05 + detail::getCriticalChance(m) + detail::getCriticalChance(cr) + statChanges.flatCriticalBonus;
    FinalStats ret;
    ret.forceTechCritChance = critChance;
    ret.meleeRangeCritChance = critChance;
    double critMultiplier = 0.5 + detail::getCriticalMultiplier(cr) + statChanges.flatCriticalMultiplierBonus;

    ret.meleeRangeCritMultiplier = critMultiplier;
    ret.forceTechCritMultiplier = critMultiplier;

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
    return ret;
}
} // namespace Simulator
