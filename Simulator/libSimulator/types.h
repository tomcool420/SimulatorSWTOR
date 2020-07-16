#pragma once
#include "detail/units.h"

namespace Simulator {
using WeaponDamage = std::pair<double, double>;

struct RawStats {
    Mastery master{0.0};
    AlacrityRating alacrityRating{0.0};
    CriticalRating criticalRating{0.0};
    Power power{0.0};
    FTPower forceTechPower{0.0};
    AccuracyRating accuracyRating{0.0};
    WeaponDamage weaponDamageMH{0.0, 0.0};
    WeaponDamage weaponDamageOH{0.0, 0.0};
    bool hasOffhand{false};
};
struct StatChanges {
    double masteryMultiplierBonus{0.0};
    Mastery masteryBonus{0.0};
    CriticalRating criticalRatingBonus{0.0};
    AlacrityRating alacrityRatingBonus{0.0};
    Power powerBonus;
    double bonusDamageMultiplier{0.0};
    double flatAlacrityBonus{0.0};
    double flatCriticalBonus{0.0};
    double flatCriticalMultiplierBonus{0.0};
    double powerMultiplier{0.0};
};
struct FinalStats {
    double meleeRangeCritChance;
    double forceTechCritChance;
    double meleeRangeCritMultiplier;
    double forceTechCritMultiplier;
    double alacrity;
    double meleeRangeBonusDamage;
    double forceTechBonusDamage;
    double bonusHealing;
    double accuracy{1.1};
    WeaponDamage weaponDamageMH{0.0, 0.0};
    WeaponDamage weaponDamageOH{0.0, 0.0};
    double multiplier{0.0};
    bool hasOffhand{false};
    double armorPen{0.0};
};

StatChanges operator+(const StatChanges &a, const StatChanges &b);
void operator+=(StatChanges &a, const StatChanges &b);
FinalStats getFinalStats(const RawStats &rawStats, const StatChanges &finalStats);

} // namespace Simulator
