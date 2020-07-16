#pragma once
#include "TimedStatusEffect.h"
namespace Simulator {
class Target;
class Debuff : public TimedStatusEffect {
    virtual void onAbilityHit(const DamageHits &hits, const Second &time, Target & target){}
};
} // namespace Simulator
