#include "types.h"
#include "Target.h"
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

    ADD_SC(castTime);
    ADD_SC(flatMeleeRangeAccuracy);
    ADD_SC(flatForceTechAccuracy);
    a.armorDebuff = a.armorDebuff || b.armorDebuff;
}

FinalStats getFinalStats(const RawStats &rawStats, const StatChanges &statChanges) {
    Mastery m = (rawStats.master + statChanges.masteryBonus) * (1 + statChanges.masteryMultiplierBonus);
    CriticalRating cr = (rawStats.criticalRating + statChanges.criticalRatingBonus);
    double critChance = 0.05 + detail::getCriticalChance(m) + detail::getCriticalChance(cr);
    FinalStats ret;
    ret.forceTechCritChance = critChance + statChanges.flatMeleeRangeCritChance;
    ret.meleeRangeCritChance = critChance + statChanges.flatForceTechCritChance;

    ret.meleeRangeCritMultiplier =
        0.5 + detail::getCriticalMultiplier(cr) + statChanges.flatMeleeRangeCriticalMultiplierBonus;
    ret.forceTechCritMultiplier =
        0.5 + detail::getCriticalMultiplier(cr) + statChanges.flatForceTechCriticalMultiplierBonus;

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
    ret.multiplier = statChanges.multiplier;
    ret.armorDebuff = statChanges.armorDebuff;
    return ret;
}

AllFinalStats getAllFinalStats(const Ability &ability, const TargetPtr &source, const TargetPtr &target) {
    if (ability.getCoefficients().empty()) {
        // if an ability has no coefficients, do the math to check for alacrity
        AbilityInfo ablInfo = ability.getInfo();
        ablInfo.coefficients = {AbilityCoefficients{}};
        auto tabl = Ability(ability.getId(), ablInfo);
        return getAllFinalStats(tabl, source, target);
    }
    auto &&rs = source->getRawStats();
    AllStatChanges scb = source->getStatChangesFromBuffs(ability, target);
    AllStatChanges scd = target ? target->getStatChangesFromDebuff(ability, source) : AllStatChanges(scb.size());
    const auto &sca = ability.getStatChanges();
    CHECK(scb.size() == scd.size());
    AllFinalStats ret(scb.size());
    for (int ii = 0; ii < scb.size(); ++ii) {
        ret[ii] = getFinalStats(rs, scb[ii] + scd[ii] + sca[ii]);
    }
    return ret;
}

void to_json(nlohmann::json &j, const RawStats &s) {
    j[key_mastery] = s.master.getValue();
    j[key_alacrity_rating] = s.alacrityRating.getValue();
    j[key_critical_rating] = s.criticalRating.getValue();
    j[key_power] = s.power.getValue();
    j[key_tech_power] = s.forceTechPower.getValue();
    j[key_force_power] = s.forceTechPower.getValue();
    j[key_mainhand_damage] = s.weaponDamageMH;
    j[key_offhand_damage] = s.weaponDamageOH;
    j[key_health] = s.hp.getValue();
    j[key_armor] = s.armor;
    j[key_has_offhand] = s.hasOffhand;
}
void from_json(const nlohmann::json &j, RawStats &s) {
    s.master = j.at(key_mastery);
    s.alacrityRating = j.at(key_alacrity_rating);
    s.criticalRating = j.at(key_critical_rating);
    s.power = j.at(key_power);
    FTPower fp = j.at(key_force_power);
    FTPower pp = j.at(key_tech_power);
    s.forceTechPower = std::max(fp, fp);
    s.weaponDamageMH = j.at(key_mainhand_damage);
    s.weaponDamageOH = j.at(key_offhand_damage);
    s.hp = j.at(key_health);
    s.armor = j.at(key_armor);
    s.hasOffhand = j.at(key_has_offhand);
}
} // namespace Simulator
