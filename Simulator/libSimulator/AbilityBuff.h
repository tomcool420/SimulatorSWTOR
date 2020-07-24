#pragma once
#include "Ability.h"
#include "Buff.h"
#include "Target.h"
#include "detail/log.h"
#include "utility.h"
namespace Simulator {
class RawSheetBuff : public Buff {
  public:
    RawSheetBuff(const std::string &buffName, const AbilityIds &ids, double rawDamageMultipler, double flatCritBonus,
                 double flatCritMultiplier, double ap);
    RawSheetBuff(const std::string &buffName, const AbilityIds &ids, const StatChanges &changes)
        : _ids(ids), _statChanges(changes), _name(buffName) {}
    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr &target) const final;
    virtual ~RawSheetBuff() = default;
    [[nodiscard]] Buff *clone() const override { return new RawSheetBuff(*this); }

  private:
    AbilityIds _ids;
    StatChanges _statChanges;
    std::string _name;
};

class RelicProcBuff : public Buff {
  public:
    RelicProcBuff(AbilityId procId, Mastery mastery, Power power, CriticalRating crit)
        : _procId(procId), _mastery(mastery), _power(power), _criticalRating(crit) {}
    [[nodiscard]] DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &player,
                                          const TargetPtr & /*target*/) override;

    SIMULATOR_SET_MACRO(BuffDuration, Second, Second{6});
    SIMULATOR_SET_MACRO(BuffCooldown, Second, Second{20});
    [[nodiscard]] Buff *clone() const override { return new RelicProcBuff(*this); }

  private:
    std::optional<Second> _nextAvailableProc;
    AbilityId _procId;
    Mastery _mastery{0.0};
    Power _power{0.0};
    CriticalRating _criticalRating{0.0};
};

class DamageTypeBuff : public Buff {
  public:
    DamageTypeBuff(const std::string &buffName, const std::vector<DamageType> &damageTypes, double rawDamageMultipler,
                   double flatCritBonus, double flatCritMultiplier, double ap);
    DamageTypeBuff(const std::string &buffName, const std::vector<DamageType> &damageTypes, StatChanges &changes)
        : _types(damageTypes), _statChanges(changes), _name(buffName) {}
    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr &target) const final;
    [[nodiscard]] Buff *clone() const override { return new DamageTypeBuff(*this); }

  private:
    std::vector<DamageType> _types;
    StatChanges _statChanges;
    std::string _name;
};
class AmplifierBuff : public Buff {
  public:
    AmplifierBuff() = default;
    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr &target) const final;
    SIMULATOR_SET_MACRO(PeriodicIntensityBonus, double, 0.0);
    SIMULATOR_SET_MACRO(WeaponExpertiseBonus, double, 0.0);
    SIMULATOR_SET_MACRO(ForcecTechWizardryBonus, double, 0.0);
    SIMULATOR_SET_MACRO(ArmorPenBonus, double, 0.0);
    [[nodiscard]] Buff *clone() const override { return new AmplifierBuff(*this); }
};

template <class T> class OnAbilityHitBuff : public Buff {
  public:
    OnAbilityHitBuff(const std::string &buffName, T &&call) : _name(buffName), _lambda(std::forward<T>(call)) {}
    [[nodiscard]] DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &source,
                                          const TargetPtr &target) override {
        return _lambda(hits, time, source, target);
    }
    [[nodiscard]] Buff *clone() const override { return new OnAbilityHitBuff(*this); }

  private:
    std::string _name;
    T _lambda;
};

template <class Lambda> OnAbilityHitBuff<Lambda> *MakeOnAbilityHitBuff(std::string name, Lambda &&t) {
    return new OnAbilityHitBuff<Lambda>(name, std::forward<Lambda>(t));
}

} // namespace Simulator
