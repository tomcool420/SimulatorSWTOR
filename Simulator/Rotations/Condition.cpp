#include "Condition.h"

namespace Simulator {
Condition getCooldownFinishedCondition(AbilityId id) {
    Condition lambda = [=](const TargetPtr &source, const TargetPtr &, const Second &, const Second &nextGCD) -> bool {
        auto hasCD = source->getAbilityCooldownEnd(id);
        if (hasCD)
            return *hasCD <= nextGCD + Second(1e-7);
        return true;
    };
    return lambda;
}

} // namespace Simulator