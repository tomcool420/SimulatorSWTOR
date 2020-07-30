#include "StaticRotation.h"

namespace Simulator {
RotationalReturn StaticRotation::getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                const Second &nextInstant, const Second &nextGCD) {
    if (_index >= _rotation.size()) {
        _index = 0;
    }
    auto &&elt = _rotation[_index++];
    if (std::holds_alternative<AbilityId>(elt)) {
        return std::get<AbilityId>(elt);
    } else if (std::holds_alternative<RotationalPriorityListPtr>(elt)) {
        return std::get<RotationalPriorityListPtr>(elt)->getNextAbility(source, target, nextInstant, nextGCD);
    } else {
        return std::get<Second>(elt);
    }
}
void StaticRotation::log(std::ostream &s, int indent) const {
    s << fmt::format("Static Rotation with size {} and current index {}:\n", _rotation.size(), _index);
    for (int ii = 0; ii < _rotation.size(); ++ii) {
        s << fmt::format("{:>{}} ", ii == _index ? "*" : "", indent + 2);
        auto &&elt = _rotation[ii];
        if (auto a = std::get_if<AbilityId>(&elt)) {
            s << fmt::format("Ability with id {}\n", *a);
        } else if (auto rpl = std::get_if<RotationalPriorityListPtr>(&elt)) {
            (*rpl)->log(s, indent + 2);
        } else if (auto t = std::get_if<Second>(&elt)) {
            s << fmt::format("Delay of {}s\n", t->getValue());
        } else {
            CHECK(false, "Static Rotation, don't know this variant type");
        }
    }
}
nlohmann::json StaticRotation::serialize() const {
    nlohmann::json obj;
    obj["type"] = 1;
    nlohmann::json j_list = nlohmann::json::array();
    for (auto &&p : _rotation) {
        nlohmann::json ip;
        if (auto aid = std::get_if<AbilityId>(&(p))) {
            ip["ability"] = *aid;
        } else if (auto s = std::get_if<Second>(&p)) {
            ip["delay"] = s->getValue();
        } else {
            auto rpl = std::get<RotationalPriorityListPtr>(p);
            ip["other"] = rpl->serialize();
        }
        j_list.push_back(ip);
    }
    obj["rotation"] = j_list;
    return obj;
}
} // namespace Simulator