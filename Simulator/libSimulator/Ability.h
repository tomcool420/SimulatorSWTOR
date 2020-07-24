#pragma once
#include "OnHitAction.h"
#include "types.h"
#include <cstdint>
#include <functional>
#include <vector>
#include "utility.h"

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
    bool isBleedDamage{false};
};
using AllAbilityCoefficient = std::vector<AbilityCoefficients>;
enum class AbilityCastType { Instant, Cast, Channeled, OffGCD };
struct AbilityInfo {
    AllAbilityCoefficient coefficients;
    AbilityCastType type{AbilityCastType::Instant};
    Second time{Second(1.5)};
    int nTicks{1};
    bool extraInitialTick{false};
};

using OnEndAction = std::function<void(const TargetPtr &, const TargetPtr &, const Second &)>;
using OnEndActions = std::vector<OnEndAction>;
class Ability {
  public:
    Ability(AbilityId iid, double coeff, double ShxMin, double ShxMax, double Am, DamageType dt, bool dot, bool aoe,
            bool offhandHit = false)
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
        _allStatChanges.resize(_info.coefficients.size());
    }
    Ability(AbilityId iid, AbilityInfo info) : _id(iid), _info(std::move(info)) {_allStatChanges.resize(_info.coefficients.size());}

    const AbilityId &getId() const { return _id; };
    const AllAbilityCoefficient &getCoefficients() const { return _info.coefficients; }
    const AbilityInfo &getInfo() const { return _info; }
    void setInfo(AbilityInfo info) {_info=std::move(info);}
    void addOnHitAction(const OnHitActionPtr &oha);
    void onAbilityHitTarget(const DamageHits &hits, const TargetPtr &source, const TargetPtr &target,
                            const Second &time);
    void addOnEndAction(OnEndAction &&action) { _onEndActions.push_back(std::move(action)); }
    void onAbilityEnd(const TargetPtr &source, const TargetPtr &target, const Second &time);
    void setStatChanges(AllStatChanges && allStatChanges){_allStatChanges=std::move(allStatChanges);}
    void setStatChanges(const StatChanges & statChanges){_allStatChanges=AllStatChanges(_info.coefficients.size(),statChanges);}
    const AllStatChanges & getStatChanges() const {return _allStatChanges;}
    SIMULATOR_SET_MACRO(Cooldown,Second,Second(0.0));
    SIMULATOR_SET_MACRO(CooldownIsAffectedByAlacrity,bool,true);
  private:
    AbilityId _id;
    AbilityInfo _info;
    OnHitActionPtrs _onHitActions;
    OnEndActions _onEndActions;
    AllStatChanges _allStatChanges;
};

[[nodiscard]] DamageRanges calculateDamageRange(const Ability &ability, const AllFinalStats &stats);
[[nodiscard]] DamageHits adjustForHitsAndCrits(const DamageRanges &hits, const AllFinalStats &stats,
                                               const TargetPtr &t);
[[nodiscard]] DamageHits adjustForDefensives(const DamageHits &hits, const AllFinalStats &stats, const TargetPtr &t);

// This does the first three
[[nodiscard]] DamageHits getHits(const Ability &ability, const AllFinalStats &stats, const TargetPtr &t);

DamageHits adjustDamageForPlayerBuffs(const DamageHits &hits, const TargetPtr &player, const TargetPtr &t);
double getRandomValue(double min, double max);
void applyDamageToTarget(DamageHits hits, const TargetPtr &source, const TargetPtr &target, const Second &time);
} // namespace Simulator
