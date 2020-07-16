#pragma once
#include "types.h"
#include <cstdint>
#include <vector>
namespace Simulator {
class Target;
using AbilityId = uint64_t;
using AbilityIds = std::vector<AbilityId>;
enum class DamageType { Kinetic = 1, Energy = 2, Internal = 3, Elemental = 4, Weapon = 5 };
class Ability {
  public:
    Ability(AbilityId iid, double coeff, double ShxMin, double ShxMax, double Am, DamageType dt, bool dot, bool aoe)
        : id(iid), coefficient(coeff), StandardHealthPercentMin(ShxMin), StandardHealthPercentMax(ShxMax),
          AmountModifierPercent(Am), damageType(dt), isDamageOverTime(dot), isAreaOfEffect(aoe){};
    AbilityId id{0};
    double coefficient{0.0};
    double StandardHealthPercentMax{0.0};
    double StandardHealthPercentMin{0.0};
    double AmountModifierPercent{0.0};
    DamageType damageType{false};
    bool isDamageOverTime{false};
    bool isAreaOfEffect{false};
};
using DamageRange = std::pair<double, double>;
struct DamageHit {
    AbilityId id;
    DamageType dt;
    std::pair<double, double> dmg;
    bool offhand{false};
    bool crit{false};
    bool miss{false};
};
using DamageHits = std::vector<DamageHit>;
// this is ignoring misses and crits
DamageHits calculateDamageRange(const Ability &ability, const FinalStats &stats);
DamageHits adjustForHitsAndCrits(const DamageHits & hits, const FinalStats &stats, const Target &t);
} // namespace Simulator

