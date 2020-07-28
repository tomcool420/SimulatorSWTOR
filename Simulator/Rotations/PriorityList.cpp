#include "PriorityList.h"

namespace Simulator {
void PriorityList::log(std::ostream &stream, int indent) {
    stream << fmt::format("Priority list with {} items\n", _priorites.size());
}
RotationalReturn PriorityList::getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                              const Second &nextInstant, const Second &nextGCD) {
    for (auto &&p : _priorites) {
        bool satisfiedConditions = true;
        for (auto &&c : p.conditions) {
            if (!c(source, target, nextInstant, nextGCD)) {
                satisfiedConditions = false;
                break;
            }
        }
        if (satisfiedConditions) {
            if (auto id = std::get_if<AbilityId>(&(p.action))) {
                return *id;
            } else if (auto rpl = std::get_if<RotationalPriorityListPtr>(&(p.action))) {
                return (*rpl)->getNextAbility(source, target, nextInstant, nextGCD);
            }
        }
    }
    CHECK(false, "No Abilities left in priority list. it is ill formed");
    return 0;
}
} // namespace Simulator