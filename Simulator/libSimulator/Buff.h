#pragma once
#include "TimedStatusEffect.h"
#include "Ability.h"

namespace Simulator {
class Player;
class Target;
class Buff : public TimedStatusEffect {
    virtual void onAbilityUsed(const Ability & ability, const Second &time, Player & player) {}
    virtual void onAbilityHit(const DamageHits &hits, const Second &time, Player & player, Target & target){}
    virtual void onAbilityEnd(const Ability & ability, const Second & time, Player &player){}
};

} // namespace Simulator
