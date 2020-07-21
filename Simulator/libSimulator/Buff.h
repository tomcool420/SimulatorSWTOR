#pragma once
#include "Ability.h"
#include "TimedStatusEffect.h"

namespace Simulator {
class Target;
class Buff : public TimedStatusEffect {
  public:
    virtual void onAbilityUsed(const Ability & /*ability*/, const Second & /*time*/, const TargetPtr & /*player*/,
                               const TargetPtr & /*target*/) {}
    virtual DamageHits onAbilityHit(DamageHits & /*hits*/, const Second & /*time*/, const TargetPtr & /*player*/,
                                    const TargetPtr & /*target*/) {
        return {};
    }
    virtual void onAbilityEnd(const Ability & /*ability*/, const Second & /*time*/, const TargetPtr & /*player*/) {}
    virtual void apply(const Ability & /*ability*/, StatChanges & /*fstats*/, const TargetPtr & /*target*/) const {}
    virtual ~Buff() = default;
    AbilityId getId() const { return _id; }

  private:
    AbilityId _id;
};
using BuffPtr = std::unique_ptr<Buff>;

} // namespace Simulator
