#include "StaticRotation.h"

namespace Simulator {
RotationalReturn StaticRotation::getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                const Second &nextInstant, const Second &nextGCD) {
    if (_index >= _rotation.size()) {
        _index = 0;
    }
    auto &&elt = _rotation[_index];
    if (std::holds_alternative<AbilityId>(elt)) {
        return std::get<AbilityId>(elt);
    } else if (std::holds_alternative<RotationalPriorityListPtr>(elt)) {
        return std::get<RotationalPriorityListPtr>(elt)->getNextAbility(source, target, nextInstant, nextGCD);
    } else {
        return std::get<Second>(elt);
    }
}
void StaticRotation::log(std::ostream &s, int indent) {
    s << fmt::format("Static Rotation with size {} and current index {}\n", _rotation.size(), _index);
    for (int ii = 0; ii < _rotation.size(); ++ii) {
        s << fmt::format("{<:{}}\n", ii == _index ? "*" : "", indent + 2);
    }
}

} // namespace Simulator