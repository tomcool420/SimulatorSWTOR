#pragma once
#include "Ability.h"
#include "TimedStatusEffect.h"

namespace Simulator {
uint64_t getNextFreeId() ;

class Target;
class Buff : public TimedStatusEffect {
  public:
    Buff(){
        _id = getNextFreeId();
    }
    virtual void onAbilityUsed(const Ability & /*ability*/, const Second & /*time*/, const TargetPtr & /*player*/,
                               const TargetPtr & /*target*/) {}
    virtual DamageHits onAbilityHit(DamageHits & /*hits*/, const Second & /*time*/, const TargetPtr & /*player*/,
                                    const TargetPtr & /*target*/) {
        return {};
    }
    virtual void onAbilityEnd(const Ability & /*ability*/, const Second & /*time*/, const TargetPtr & /*player*/) {}
    virtual void apply(const Ability & /*ability*/, AllStatChanges & /*fstats*/, const TargetPtr & /*target*/) const {}
    virtual ~Buff() = default;
    AbilityId getId() const { return _id; }
    void setId(AbilityId id) {_id=id;}
  private:
    AbilityId _id{0};
};
using BuffPtr = std::unique_ptr<Buff>;

} // namespace Simulator
