#pragma once
#include "Simulator/libSimulator/Target.h"
#include "Simulator/libSimulator/types.h"
#include <nlohmann/json.hpp>

namespace Simulator {
class ConditionC {
  public:
    virtual bool check(const TargetPtr &, const TargetPtr &, const Second &, const Second &) = 0;
    virtual nlohmann::json serialize() = 0;
};

constexpr char cooldown_condition[] = "cooldown_condition";
constexpr char energy_condition[] = "energy_condition";
constexpr char buff_condition[] = "buff_condition";
constexpr char debuff_condition[] = "debuff_condition";
constexpr char sub30_condition[] = "sub30_condition";

class CooldownCondition : public ConditionC {
  public:
    CooldownCondition(AbilityId id) : _id(id) {}
    CooldownCondition(const nlohmann::json &json);
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;

  private:
    AbilityId _id;
};

class EnergyCondition : public ConditionC {
  public:
    EnergyCondition(const nlohmann::json &json);
    EnergyCondition(double energy, bool above = true) : _energy(energy), _above(above) {}
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;

  private:
    double _energy{0};
    bool _above{true};
};

class BuffCondition : public ConditionC {
  public:
    BuffCondition(const nlohmann::json &json);
    BuffCondition(AbilityId id, Second timeRemaining) : _buffId(id), _timeRemaing(timeRemaining) {}
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;

  private:
    AbilityId _buffId;
    Second _timeRemaing{0.0};
};

class SubThirtyCondition : public ConditionC {
  public:
    SubThirtyCondition(const nlohmann::json &json);
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;
};

using ConditionPtr = std::unique_ptr<ConditionC>;
using Conditions = std::vector<ConditionPtr>;
ConditionPtr getCooldownFinishedCondition(AbilityId id);

} // namespace Simulator