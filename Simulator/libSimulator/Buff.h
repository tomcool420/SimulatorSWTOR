#pragma once
#include "Ability.h"
#include "TimedStatusEffect.h"

namespace Simulator {
class Player;
class Target;
class Buff : public TimedStatusEffect {
public:
    virtual void onAbilityUsed(const Ability &/*ability*/, const Second &/*time*/, Player &/*player*/, Target &/*target*/) {}
    virtual void onAbilityHit(const DamageHits &/*hits*/, const Second &/*time*/, Player &/*player*/, Target &/*target*/) {}
    virtual void onAbilityEnd(const Ability &/*ability*/, const Second &/*time*/, Player &/*player*/) {}
    virtual void apply(const Ability &/*ability*/, StatChanges &/*fstats*/, const Target &/*target*/)const {}
    virtual ~Buff() = default;

};
using BuffPtr = std::unique_ptr<Buff>;

} // namespace Simulator
