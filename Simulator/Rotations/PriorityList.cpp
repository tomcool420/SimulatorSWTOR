#include "PriorityList.h"

namespace Simulator {
void PriorityList::log(std::ostream &stream, int) const {
    stream << fmt::format("Priority list with {} items\n", _priorites.size());
}
RotationalReturn PriorityList::getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                              const Second &nextInstant, const Second &nextGCD) {
    for (auto &&p : _priorites) {
        bool satisfiedConditions = true;
        for (auto &&c : p.conditions) {
            if (!c->check(source, target, nextInstant, nextGCD)) {
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
nlohmann::json PriorityList::serialize() const {
    nlohmann::json obj;
    obj["type"] = 0;
    nlohmann::json j_list = nlohmann::json::array();
    for (auto &&p : _priorites) {
        nlohmann::json ip;
        if (auto aid = std::get_if<AbilityId>(&(p.action))) {
            ip["ability"] = *aid;
        } else {
            auto rpl = std::get<RotationalPriorityListPtr>(p.action);
            ip["other"] = rpl->serialize();
        }
        ip["conditions"] = nlohmann::json::array();
        j_list.push_back(ip);
    }
    obj["priority"] = j_list;
    return obj;
}
} // namespace Simulator