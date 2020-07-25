#pragma once
#include "constants.h"
#include "detail/units.h"
#include <vector>

namespace Simulator {
class Target;
using TargetPtr = std::shared_ptr<Target>;
using TargetPtrs = std::vector<TargetPtr>;
class Ability;
using AbilityPtr = std::shared_ptr<Ability>;
class Debuff;
using DebuffPtr = std::unique_ptr<Debuff>;
class Buff;
using BuffPtr = std::unique_ptr<Buff>;

using WeaponDamage = std::pair<double, double>;
enum class DamageType { Kinetic = 1, Energy = 2, Internal = 3, Elemental = 4, Weapon = 5 };
using DamageTypes = std::vector<DamageType>;
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

    Second castTime{0.0};
    bool armorDebuff{false};
    double flatMeleeRangeAccuracy{0.0};
    double flatForceTechAccuracy{0.0};
};
using AllStatChanges = std::vector<StatChanges>;

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
    bool armorDebuff{false};
};
using AllFinalStats = std::vector<FinalStats>;
StatChanges operator+(const StatChanges &a, const StatChanges &b);
void operator+=(StatChanges &a, const StatChanges &b);
FinalStats getFinalStats(const RawStats &rawStats, const StatChanges &finalStats);
AllFinalStats getAllFinalStats(const Ability &ability, const TargetPtr &player, const TargetPtr &target);

using AbilityId = uint64_t;
using AbilityIds = std::vector<AbilityId>;

struct DamageRange {
    AbilityId id;
    DamageType dt;
    std::pair<double, double> dmg;
    bool offhand{false};
    bool aoe{false};
};
using DamageRanges = std::vector<DamageRange>;
struct DamageHit {
    AbilityId id;
    DamageType dt;
    double dmg;
    bool offhand{false};
    bool crit{false};
    bool miss{false};
    bool aoe{false};
};
using DamageHits = std::vector<DamageHit>;

} // namespace Simulator
