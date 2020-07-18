#include "DOT.h"
#include "Target.h"

namespace Simulator {
DamageHits DOT::tick(const Target &t, const FinalStats &s, const Second &time) {
    _lastTickTime = time;
    ++_tickCount;
    auto damageRange = calculateDamageRange(*this, s);
    auto damageHits = adjustForHitsAndCrits(damageRange, s, t);
    return adjustForDefensives(damageHits, s, t);
}
} // namespace Simulator
