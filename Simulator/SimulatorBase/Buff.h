#pragma once
#include "Ability.h"
#include "Debuff.h"
#include "TimedStatusEffect.h"

namespace Simulator {
uint64_t getNextFreeId();

class Target;
class Buff : public TimedStatusEffect {
  public:
    Buff() { _id = getNextFreeId(); }
    virtual void onAbilityUsed(const Ability & /*ability*/, const Second & /*time*/, const TargetPtr & /*player*/,
                               const TargetPtr & /*target*/) {}
    [[nodiscard]] virtual DamageHits onAbilityHit(DamageHits & /*hits*/, const Second & /*time*/,
                                                  const TargetPtr & /*player*/, const TargetPtr & /*target*/) {
        return {};
    }
    [[nodiscard]] virtual DebuffEvents resolveEventsUpToTime(const Second &time, const TargetPtr &);
    [[nodiscard]] std::optional<Second> getNextEventTime() const override;
    // a hook to allow some buffs to have an on activation state (force clarity and smugglers luck)
    virtual void activate(const Second & /*time*/) {}
    virtual void onAbilityEnd(const Ability & /*ability*/, const Second & /*time*/, const TargetPtr & /*player*/) {}
    virtual void apply(const Ability & /*ability*/, AllStatChanges & /*fstats*/, const TargetPtr & /*target*/) const {}
    virtual void adjustEnergyAndCastTime(const Ability &abl, EnergyCost &energyCost, Second &castTime, bool &instant);
    virtual void OnAbilityHitTarget(const Ability & /*ability*/, const DamageHits & /*hits*/,
                                    const TargetPtr & /*player*/, const Second & /*time*/) const {}
    virtual ~Buff() = default;
    AbilityId getId() const { return _id; }
    void setId(AbilityId id) { _id = id; }
    [[nodiscard]] virtual Buff *clone() const { return new Buff(*this); }

    void setCurrentStacks(int currentStacks, const Second &time);

    SIMULATOR_SET_MACRO(MaxStacks, int, 0);
    SIMULATOR_GET_ONLY_MACRO(CurrentStacks, int, 0);
    SIMULATOR_SET_MACRO(StackDuration, std::optional<Second>, std::nullopt);

  private:
    AbilityId _id{0};
    std::optional<Second> _stackExpiration = std::nullopt;
};

} // namespace Simulator
