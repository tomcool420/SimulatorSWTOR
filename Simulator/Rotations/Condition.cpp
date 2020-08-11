#include "Condition.h"
#include "Simulator/SimulatorBase/Energy.h"
namespace Simulator {
Conditions getCooldownFinishedCondition(AbilityId id) {
    Conditions cc;
    cc.push_back(std::make_unique<CooldownCondition>(id));
    return cc;
}
constexpr char condition_type_key[] = "type";
constexpr char condition_amount_key[] = "amount";
constexpr char condition_invert_key[] = "invert";
constexpr char condition_id_key[] = "id";
constexpr char condition_remaining_time_key[] = "remaining_time";
namespace {
template <class T> T getValue(const nlohmann::json &j, const std::string &key) {
    CHECK(j.contains(key));
    return j[key].get<T>();
}
} // namespace
EnergyCondition::EnergyCondition(const nlohmann::json &j) {
    _energy = getValue<double>(j, condition_amount_key);
    _above = !getValue<bool>(j, condition_invert_key);
}
bool EnergyCondition::check(const TargetPtr &source, const TargetPtr &, const Second &, const Second &) {
    auto em = source->getEnergyModel();
    if (!em)
        return _above;
    return _above ? em->getCurrentEnergy() > _energy : em->getCurrentEnergy() < _energy;
}
nlohmann::json EnergyCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = energy_condition;
    j[condition_amount_key] = _energy;
    j[condition_invert_key] = !_above;
    return j;
}

CooldownCondition::CooldownCondition(const nlohmann::json &j) { _id = getValue<AbilityId>(j, condition_id_key); }
bool CooldownCondition::check(const TargetPtr &source, const TargetPtr &, const Second &, const Second &nextFreeGCD) {
    if (auto cd = source->getAbilityCooldownEnd(_id)) {
        return *cd <= nextFreeGCD;
    }
    return true;
}
nlohmann::json CooldownCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = cooldown_condition;
    j[condition_id_key] = _id;
    return j;
}

SubThirtyCondition::SubThirtyCondition(const nlohmann::json &) {}
bool SubThirtyCondition::check(const TargetPtr &, const TargetPtr &target, const Second &, const Second &) {
    return target->getCurrentHealth() / target->getMaxHealth() < 0.3;
}
nlohmann::json SubThirtyCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = sub30_condition;
    return j;
}

BuffCondition::BuffCondition(const nlohmann::json &j) {
    _buffId = getValue<AbilityId>(j, condition_id_key);
    _timeRemaing = Second(getValue<double>(j, condition_remaining_time_key));
    _invert = getValue<bool>(j, condition_invert_key);
}
bool BuffCondition::check(const TargetPtr &source, const TargetPtr &, const Second &, const Second &nextFreeGCD) {
    if (auto b = source->getBuff<Buff>(_buffId)) {
        if (b->getEndTime() > nextFreeGCD + _timeRemaing)
            return !_invert;
    }
    return _invert;
}
nlohmann::json BuffCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = buff_condition;
    j[condition_id_key] = _buffId;
    j[condition_remaining_time_key] = _timeRemaing.getValue();
    j[condition_invert_key] = _invert;
    return j;
}

DebuffCondition::DebuffCondition(const nlohmann::json &j) {
    _buffId = getValue<AbilityId>(j, condition_id_key);
    _timeRemaing = Second(getValue<double>(j, condition_remaining_time_key));
    _invert = getValue<bool>(j, condition_invert_key);
}
bool DebuffCondition::check(const TargetPtr &source, const TargetPtr &target, const Second &,
                            const Second &nextFreeGCD) {
    if (auto b = target->getDebuff<Debuff>(_buffId, source->getId())) {
        if (b->getEndTime() >= nextFreeGCD + _timeRemaing) {
            return !_invert;
        }
    }
    return _invert;
}
nlohmann::json DebuffCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = debuff_condition;
    j[condition_id_key] = _buffId;
    j[condition_remaining_time_key] = _timeRemaing.getValue();
    j[condition_invert_key] = _invert;
    return j;
}
ConditionPtr getDeserializedCondition(const nlohmann::json &j) {
    CHECK(j.contains(condition_type_key));
    auto k = j[condition_type_key].get<std::string>();
    if (k == cooldown_condition) {
        return std::make_unique<CooldownCondition>(j);
    } else if (k == energy_condition) {
        return std::make_unique<EnergyCondition>(j);
    } else if (k == buff_condition) {
        return std::make_unique<BuffCondition>(j);
    } else if (k == debuff_condition) {
        return std::make_unique<DebuffCondition>(j);
    } else if (k == sub30_condition) {
        return std::make_unique<SubThirtyCondition>(j);
    }
    return nullptr;
}

nlohmann::json serializeConditions(const Conditions &conditions) {
    nlohmann::json j = nlohmann::json::array();
    for (auto &&c : conditions) {
        j.push_back(c->serialize());
    }
    return j;
}
Conditions deserializeConditions(const nlohmann::json &ja) {
    CHECK(ja.is_array());
    Conditions c;
    for (auto &&j : ja) {
        c.push_back(getDeserializedCondition(j));
    }
    return c;
}
} // namespace Simulator
