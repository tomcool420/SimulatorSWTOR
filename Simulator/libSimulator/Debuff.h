#pragma once
#include "TimedStatusEffect.h"
namespace Simulator {
class Target;
class Debuff : public TimedStatusEffect {
    virtual void onAbilityHit(const DamageHits &/*hits*/, const Second &/*time*/, Target &/*target*/) {}
#define SIMULATOR_STATUS(effect)                                                                                       \
  public:                                                                                                              \
bool is##effect () const { return _##effect; }                                                                      \
void set##effect (bool v) { _##effect = v; }                                                                        \
                                                                                                                       \
  private:                                                                                                             \
bool _##effect{false};

    SIMULATOR_STATUS(Bleeding);
    SIMULATOR_STATUS(Burning);
    SIMULATOR_STATUS(Poisoned);
};
using DebuffPtr = std::unique_ptr<Debuff>;
} // namespace Simulator
