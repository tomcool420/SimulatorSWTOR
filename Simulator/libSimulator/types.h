#pragma once
#include "detail/units.h"
#include "constants.h"

namespace Simulator {
class Target;
class Ability;
using TargetPtr = std::shared_ptr<Target>;

using WeaponDamage = std::pair<double, double>;
enum class DamageType { Kinetic = 1, Energy = 2, Internal = 3, Elemental = 4, Weapon = 5 };

struct RawStats {
    Mastery master{0.0};
    AlacrityRating alacrityRating{0.0};
    CriticalRating criticalRating{0.0};
    Power power{0.0};
    FTPower forceTechPower{0.0};
    AccuracyRating accuracyRating{0.0};
    WeaponDamage weaponDamageMH{0.0, 0.0};
    WeaponDamage weaponDamageOH{0.0, 0.0};
    DamageType weaponDamageTypeMH{DamageType::Energy};
    DamageType weaponDamageTypeOH{DamageType::Energy};
    Armor armor{BossArmor};
    HealthPoints hp{6500000};
    bool hasOffhand{false};
    double armorPen{0.0};
};
struct StatChanges {
    double masteryMultiplierBonus{0.0};
    Mastery masteryBonus{0.0};

    CriticalRating criticalRatingBonus{0.0};
    double flatMeleeRangeCritChance{0.0};
    double flatForceTechCritChance{0.0};
    double flatMeleeRangeCriticalMultiplierBonus{0.0};
    double flatForceTechCriticalMultiplierBonus{0.0};

    Power powerBonus;
    double powerMultiplier{0.0};

    double bonusDamageMultiplier{0.0};

    AlacrityRating alacrityRatingBonus{0.0};
    double flatAlacrityBonus{0.0};

    double armorPen{0.0};

    double multiplier{0.0};
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
    DamageType weaponDamageTypeMH{DamageType::Energy};
    DamageType weaponDamageTypeOH{DamageType::Energy};
    double multiplier{0.0};
    bool hasOffhand{false};
    double armorPen{0.0};
};

StatChanges operator+(const StatChanges &a, const StatChanges &b);
void operator+=(StatChanges &a, const StatChanges &b);
FinalStats getFinalStats(const RawStats &rawStats, const StatChanges &finalStats);
FinalStats getFinalStats(const Ability &ability, const Target &player, const Target &target);

using AbilityId = uint64_t;
using AbilityIds = std::vector<AbilityId>;

struct DamageRange {
    AbilityId id;
    DamageType dt;
    std::pair<double, double> dmg;
    bool offhand{false};
};
using DamageRanges = std::vector<DamageRange>;
struct DamageHit {
    AbilityId id;
    DamageType dt;
    double dmg;
    bool offhand{false};
    bool crit{false};
    bool miss{false};
};
using DamageHits = std::vector<DamageHit>;

} // namespace Simulator
