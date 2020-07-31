#include "PriorityList.h"
#include "StaticRotation.h"

namespace Simulator {
namespace {
template <class T> T getValue(const nlohmann::json &j, const std::string &key) {
    CHECK(j.contains(key));
    return j[key].get<T>();
}
} // namespace

void PriorityList::log(std::ostream &stream, int) const {
    stream << fmt::format("Priority list with {} items\n", _priorites.size());
}

RotationalPriorityListPtr RotationalPriorityList::deserialize(const nlohmann::json &j) {
    CHECK(j.contains("type"));
    auto v = getValue<int>(j, "type");
    if (v == 0) {
        return PriorityList::deserialize(j);
    }
    return StaticRotation::deserialize(j);
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
    obj[key_type] = 0;
    nlohmann::json j_list = nlohmann::json::array();
    for (auto &&p : _priorites) {
        nlohmann::json ip;
        if (auto aid = std::get_if<AbilityId>(&(p.action))) {
            ip[key_ability] = *aid;
        } else {
            auto rpl = std::get<RotationalPriorityListPtr>(p.action);
            ip[key_other] = rpl->serialize();
        }
        ip[key_conditions] = serializeConditions(p.conditions);
        j_list.push_back(ip);
    }
    obj["priority"] = j_list;
    return obj;
}

std::shared_ptr<PriorityList> PriorityList::deserialize(const nlohmann::json &j) {
    CHECK(j.contains(key_type));
    CHECK(getValue<int>(j, key_type) == 0);
    auto rot = j.at(key_priority);
    CHECK(rot.is_array());
    auto pl = std::make_unique<PriorityList>();
    for (auto &&ro : rot) {
        if (ro.contains(key_ability)) {
            pl->addAbility(getValue<AbilityId>(ro, key_ability), deserializeConditions(ro.at(key_conditions)));
        } else {
            CHECK(ro.contains(key_other));
            pl->addPriorityList(RotationalPriorityList::deserialize(ro[key_other]),
                                deserializeConditions(ro.at(key_conditions)));
        }
    }
    return pl;
}
} // namespace Simulator