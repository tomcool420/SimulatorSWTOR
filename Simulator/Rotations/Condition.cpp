#include "Condition.h"
#include "Simulator/libSimulator/Energy.h"
namespace Simulator {
ConditionPtr getCooldownFinishedCondition(AbilityId id) { return std::make_unique<CooldownCondition>(id); }
constexpr char condition_type_key[] = "type";
constexpr char condition_amount_key[] = "amount";
constexpr char condition_invert_key[] = "invert";
constexpr char condition_id_key[] = "id";
constexpr char condition_remaining_time_key[] = "remaining_time";

bool EnergyCondition::check(const TargetPtr &source, const TargetPtr &, const Second &, const Second &) {
    auto em = source->getEnergyModel();
    if (!em)
        return true;
    return _above ? em->getCurrentEnergy() > _energy : em->getCurrentEnergy() < _energy;
}
nlohmann::json EnergyCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = energy_condition;
    j[condition_amount_key] = _energy;
    j[condition_invert_key] = !_above;
    return j;
}

bool CooldownCondition::check(const TargetPtr &source, const TargetPtr &, const Second &, const Second &nextFreeGCD) {
    if (auto cd = source->getAbilityCooldownEnd(_id)) {
        return *cd < nextFreeGCD;
    }
    return true;
}
nlohmann::json CooldownCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = cooldown_condition;
    return j;
}
bool SubThirtyCondition::check(const TargetPtr &, const TargetPtr &target, const Second &, const Second &) {
    return target->getCurrentHealth() / target->getMaxHealth() < 0.3;
}
nlohmann::json SubThirtyCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = sub30_condition;
    return j;
}

bool BuffCondition::check(const TargetPtr &source, const TargetPtr &, const Second &,
                          const Second &nextFreeGCD) {
    if (auto b = source->getBuff<Buff>(_buffId)) {
        if (b->getEndTime() > nextFreeGCD + _timeRemaing)
            return true;
    }
    return false;
}
nlohmann::json BuffCondition::serialize() {
    nlohmann::json j;
    j[condition_type_key] = sub30_condition;
    j[condition_id_key] = _buffId;
    j[condition_remaining_time_key] = _timeRemaing.getValue();
    return j;
}
} // namespace Simulator