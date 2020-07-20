#pragma once
#include "Ability.h"
#include "TimedStatusEffect.h"

namespace Simulator {
class Target;
class Buff : public TimedStatusEffect {
public:
    virtual void onAbilityUsed(const Ability &/*ability*/, const Second &/*time*/, Target &/*player*/, Target &/*target*/) {}
    virtual DamageHits onAbilityHit(DamageHits &/*hits*/, const Second &/*time*/, Target &/*player*/, Target &/*target*/) {return {};}
    virtual void onAbilityEnd(const Ability &/*ability*/, const Second &/*time*/, Target &/*player*/) {}
    virtual void apply(const Ability &/*ability*/, StatChanges &/*fstats*/, const Target &/*target*/)const {}
    virtual ~Buff() = default;
    AbilityId getId() const { return _id;}
private:
    AbilityId _id;
};
using BuffPtr = std::unique_ptr<Buff>;

} // namespace Simulator
