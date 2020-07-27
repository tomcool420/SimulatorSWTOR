#pragma once
#include "TimedStatusEffect.h"
#include "types.h"
#include "utility.h"
#include <optional>
namespace Simulator {
enum class DebuffEventType { Apply, Remove, Tick, Internal };
struct DebuffEvent {
    DebuffEventType type;
    Second time;
    DamageHits hits;
};
using DebuffEvents = std::vector<DebuffEvent>;

class Debuff : public TimedStatusEffect {
  public:
    Debuff(AbilityId id) : TimedStatusEffect(), _id(id) {}
    [[nodiscard]] virtual DamageHits onAbilityHit(DamageHits & /*hits*/, const Second & /*time*/,
                                                  const TargetPtr & /*player*/, const TargetPtr & /*target*/) {
        return {};
    }
    [[nodiscard]] virtual DebuffEvents resolveEventsUpToTime(const Second &time, const TargetPtr &) {
        if (time > getEndTime()) {
            return {{DebuffEventType::Remove}};
        }
        return {};
    }
    virtual void modifyStats(const Ability & /*ability*/, AllStatChanges & /*fstats*/,
                             const std::shared_ptr<const Target> & /*target*/) const {}
    [[nodiscard]] virtual Debuff *clone() const = 0;
    void setSource(TargetPtr source) { _source = std::move(source); }
    virtual void apply(const FinalStats &, const Second &time) { refresh(time); }
    virtual void refresh(const Second &time) { setStartTime(time); }
    [[nodiscard]] const TargetPtr &getSource() const { return _source; }
    [[nodiscard]] const AbilityId &getId() const { return _id; }
    using OnDebuffRemovedAction = std::function<void(const TargetPtr &, const TargetPtr &, const Second &)>;
    void addOnDebuffRemovedAction(OnDebuffRemovedAction &&action) { _onRemovalActions.push_back(std::move(action)); }
    void onWasRemovedFromTarget(const TargetPtr &target, const Second &time) {
        for (auto &&action : _onRemovalActions) {
            action(_source, target, time);
        }
    }
    [[nodiscard]] virtual bool getRefreshable() const { return true; }

  private:
    AbilityId _id;
    TargetPtr _source{nullptr};
    SIMULATOR_SET_MACRO(BlockedByDebuffs, AbilityIds, {})
    SIMULATOR_SET_MACRO(Bleeding, bool, false);
    SIMULATOR_SET_MACRO(Burning, bool, false);
    SIMULATOR_SET_MACRO(Poisoned, bool, false);
    SIMULATOR_SET_MACRO(Unique, bool, false);
    std::vector<OnDebuffRemovedAction> _onRemovalActions;
};

template <class T> class OnAbilityHitDebuff : public Debuff {
  public:
    OnAbilityHitDebuff(const std::string &debuffName, AbilityId id, T &&call)
        : Debuff(id), _name(debuffName), _lambda(std::forward<T>(call)) {}
    Debuff *clone() const override { return new OnAbilityHitDebuff<T>(*this); }
    [[nodiscard]] DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &source,
                                          const TargetPtr &target) override {
        return _lambda(hits, time, source, target, *this);
    }

  private:
    std::string _name;
    T _lambda;
};

template <class Lambda> OnAbilityHitDebuff<Lambda> *MakeOnAbilityHitDebuff(std::string name, AbilityId id, Lambda &&t) {
    return new OnAbilityHitDebuff<Lambda>(name, id, std::forward<Lambda>(t));
}

} // namespace Simulator
