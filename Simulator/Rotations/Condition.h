#pragma once
#include "Simulator/libSimulator/Target.h"
#include "Simulator/libSimulator/types.h"
#include <nlohmann/json.hpp>

namespace Simulator {
class ConditionC {
    virtual bool operator()(const AbilityId &id, const TargetPtr &, const TargetPtr &, const Second &, const Second &);
    virtual nlohmann::json serialize();
};

constexpr char cooldown_condition[] = "cooldown_condition";
constexpr char energy_condition[] = "energy_condition";
constexpr char buff_condition[] = "buff_condition";
constexpr char debuff_condition[] = "debuff_condition";

class CooldownCondition : public ConditionC {
    CooldownCondition(const nlohmann::json &json);
    bool operator()(const AbilityId &id, const TargetPtr &, const TargetPtr &, const Second &, const Second &);
    nlohmann::json serialize();
};

class EnergyCondition : ConditionC {
  public:
    EnergyCondition(const nlohmann::json &json);
    bool operator()(const AbilityId &id, const TargetPtr &, const TargetPtr &, const Second &, const Second &);
    nlohmann::json serialize();

  private:
    double _energy{0};
    bool _above{true};
};

class BuffCondition : ConditionC {
  public:
    BuffCondition(const nlohmann::json &json);
    bool operator()(const AbilityId &id, const TargetPtr &, const TargetPtr &, const Second &, const Second &);
    nlohmann::json serialize();

  private:
    AbilityId _buffId;
    Second _timeRemaing;
};
using Condition = std::function<bool(const TargetPtr &, const TargetPtr &, const Second &, const Second &)>;
using Conditions = std::vector<Condition>;
Condition getCooldownFinishedCondition(AbilityId id);

} // namespace Simulator