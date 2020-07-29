#include "PriorityListRotation.h"

namespace Simulator {
std::optional<Second> PriorityListRotation::getNextEventTime() {
    auto time = Rotation::getNextEventTime();
    if (time)
        return time;
    if (_nextAbility)
        return _nextAbility->second;
    bool found = false;
    do {
        auto t = _plr->getNextAbility(getSource(), getTarget(), getNextFreeGCDForInstantCast(), getNextFreeGCD());
        if (auto d = std::get_if<Second>(&t)) {
            setNextFreeGCD(getNextFreeGCD() + *d);
            SIM_INFO("Delaying for {} seconds", d->getValue());
        } else {
            auto ablid = std::get<AbilityId>(t);
            auto abl = _class->getAbility(ablid);
            auto c = getSource()->getAbilityCooldownEnd(ablid);
            Second earliest =
                abl->getInfo().type == AbilityCastType::OffGCD ? getNextFreeGCDForInstantCast() : getNextFreeGCD();
            if (c) {
                earliest = std::max(*c, earliest);
            }
            _nextAbility = {ablid, earliest};
            found = true;
        }
    } while (found == false);
    if (_nextAbility)
        return _nextAbility->second;
    return std::nullopt;
}
AbilityPtr PriorityListRotation::getNextAbility() {
    CHECK(_nextAbility);
    auto abl = _class->getAbility(_nextAbility->first);
    _nextAbility = std::nullopt;
    return abl;
}
} // namespace Simulator