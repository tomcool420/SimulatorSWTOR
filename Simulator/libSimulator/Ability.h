#pragma once
#include "OnHitAction.h"
#include "types.h"
#include <cstdint>
#include <vector>

namespace Simulator {
class Target;

struct AbilityCoefficients {
    double coefficient{0.0};
    double StandardHealthPercentMin{0.0};
    double StandardHealthPercentMax{0.0};
    double AmountModifierPercent{0.0};
    DamageType damageType{false};
    bool isDamageOverTime{false};
    bool isAreaOfEffect{false};
};

class Ability {
  public:
    Ability(AbilityId iid, double coeff, double ShxMin, double ShxMax, double Am, DamageType dt, bool dot, bool aoe)
        : _id(iid) {
        _coefficients.coefficient = coeff;
        _coefficients.StandardHealthPercentMin = ShxMin;
        _coefficients.StandardHealthPercentMax = ShxMax;
        _coefficients.AmountModifierPercent = Am;
        _coefficients.damageType = dt;
        _coefficients.isDamageOverTime = dot;
        _coefficients.isAreaOfEffect = aoe;
    };
    Ability(AbilityId iid, AbilityCoefficients coefficients) : _id(iid), _coefficients(coefficients) {}

    const AbilityId &getId() const { return _id; };
    const AbilityCoefficients getCoefficients() const { return _coefficients; }
    void addOnHitAction(const OnHitActionPtr & oha);
    void onAbilityHitTarget(const DamageHits &hits,const TargetPtr &source, const TargetPtr &target, const Second &time);
  private:
    AbilityId _id;
    AbilityCoefficients _coefficients;
    OnHitActionPtrs _onHitActions;
};
using AbilityPtr = std::shared_ptr<Ability>;

[[nodiscard]] DamageRanges calculateDamageRange(const Ability &ability, const FinalStats &stats);
[[nodiscard]] DamageHits adjustForHitsAndCrits(const DamageRanges &hits, const FinalStats &stats, const Target &t);
[[nodiscard]] DamageHits adjustForDefensives(const DamageHits &hits, const FinalStats &stats, const Target &t);

//This does the first three
[[nodiscard]] DamageHits getHits(const Ability & ability, const FinalStats & stats, const Target &t);


DamageHits adjustDamageForPlayerBuffs(const DamageHits &hits, Target &player, Target &t);
double getRandomValue(double min, double max);
void applyDamageToTarget(DamageHits hits, Target & source, Target & target,const Second& time);
} // namespace Simulator
