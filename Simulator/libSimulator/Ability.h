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
    DamageType damageType{DamageType::Weapon};
    bool isDamageOverTime{false};
    bool isAreaOfEffect{false};
    bool respectsGCD{true};
    double multiplier{0.00};
    bool isOffhandHit{false};
};
using AllAbilityCoefficient = std::vector<AbilityCoefficients>;
struct AbilityInfo {
    AllAbilityCoefficient coefficients;
};

class Ability {
  public:
    Ability(AbilityId iid, double coeff, double ShxMin, double ShxMax, double Am, DamageType dt, bool dot, bool aoe, bool offhandHit=false)
        : Ability(iid, AbilityCoefficients{coeff, ShxMin, ShxMax, Am, dt, dot, aoe}, offhandHit){};
    Ability(AbilityId iid, AbilityCoefficients coefficients, bool hasOffhandHit = false) : _id(iid) {
        _info.coefficients.push_back(coefficients);
        if (coefficients.damageType == DamageType::Weapon && hasOffhandHit) {
            coefficients.StandardHealthPercentMin = 0.0;
            coefficients.coefficient = 0.0;
            coefficients.StandardHealthPercentMax = 0.0;
            coefficients.isOffhandHit = true;
            _info.coefficients.push_back(coefficients);
        }
    }
    Ability(AbilityId iid, AbilityInfo info) : _id(iid), _info(std::move(info)) {}

    const AbilityId &getId() const { return _id; };
    const AllAbilityCoefficient &getCoefficients() const { return _info.coefficients; }
    const AbilityInfo &getInfo() const { return _info; }
    void addOnHitAction(const OnHitActionPtr &oha);
    void onAbilityHitTarget(const DamageHits &hits, const TargetPtr &source, const TargetPtr &target,
                            const Second &time);

  private:
    AbilityId _id;
    AbilityInfo _info;
    OnHitActionPtrs _onHitActions;
};
using AbilityPtr = std::shared_ptr<Ability>;

[[nodiscard]] DamageRanges calculateDamageRange(const Ability &ability, const AllFinalStats &stats);
[[nodiscard]] DamageHits adjustForHitsAndCrits(const DamageRanges &hits, const AllFinalStats &stats, const TargetPtr &t);
[[nodiscard]] DamageHits adjustForDefensives(const DamageHits &hits, const AllFinalStats &stats, const TargetPtr &t);

// This does the first three
[[nodiscard]] DamageHits getHits(const Ability &ability, const AllFinalStats &stats, const TargetPtr &t);

DamageHits adjustDamageForPlayerBuffs(const DamageHits &hits, const TargetPtr &player, const TargetPtr &t);
double getRandomValue(double min, double max);
void applyDamageToTarget(DamageHits hits, const TargetPtr &source, const TargetPtr &target, const Second &time);
} // namespace Simulator
