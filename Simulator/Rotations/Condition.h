#pragma once
#include "Simulator/SimulatorBase/Target.h"
#include "Simulator/SimulatorBase/types.h"
#include <nlohmann/json.hpp>

namespace Simulator {
class ConditionC {
  public:
    virtual bool check(const TargetPtr &, const TargetPtr &, const Second &, const Second &) = 0;
    virtual nlohmann::json serialize() = 0;
    virtual ~ConditionC() = default;
};

constexpr char cooldown_condition[] = "cooldown_condition";
constexpr char energy_condition[] = "energy_condition";
constexpr char buff_condition[] = "buff_condition";
constexpr char debuff_condition[] = "debuff_condition";
constexpr char sub30_condition[] = "sub30_condition";

class CooldownCondition : public ConditionC {
  public:
    CooldownCondition(AbilityId id) : _id(id) {}
    explicit CooldownCondition(const nlohmann::json &json);
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;
    virtual ~CooldownCondition() = default;

  private:
    AbilityId _id;
};

class EnergyCondition : public ConditionC {
  public:
    explicit EnergyCondition(const nlohmann::json &json);
    EnergyCondition(double energy, bool above = true) : _energy(energy), _above(above) {}
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;
    virtual ~EnergyCondition() = default;

  private:
    double _energy{0};
    bool _above{true};
};

class BuffCondition : public ConditionC {
  public:
    explicit BuffCondition(const nlohmann::json &json);
    BuffCondition(AbilityId id, Second timeRemaining, bool invert = false)
        : _buffId(id), _timeRemaing(timeRemaining), _invert(invert) {}
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;
    virtual ~BuffCondition() = default;

  private:
    AbilityId _buffId;
    Second _timeRemaing{0.0};
    bool _invert{false};
};

class DebuffCondition : public ConditionC {
  public:
    explicit DebuffCondition(const nlohmann::json &json);
    DebuffCondition(AbilityId id, Second timeRemaining, bool invert = false)
        : _buffId(id), _timeRemaing(timeRemaining), _invert(invert) {}
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;
    virtual ~DebuffCondition() = default;

  private:
    AbilityId _buffId;
    Second _timeRemaing{0.0};
    bool _invert{false};
};

class SubThirtyCondition : public ConditionC {
  public:
    SubThirtyCondition() = default;
    explicit SubThirtyCondition(const nlohmann::json &json);
    bool check(const TargetPtr &source, const TargetPtr &target, const Second &nextFreeInstant,
               const Second &nextFreeGCD) override;
    nlohmann::json serialize() override;
    virtual ~SubThirtyCondition() = default;
};

using ConditionPtr = std::unique_ptr<ConditionC>;
using Conditions = std::vector<ConditionPtr>;
Conditions getCooldownFinishedCondition(AbilityId id);
ConditionPtr getDeserializedCondition(const nlohmann::json &condition);

nlohmann::json serializeConditions(const Conditions &conditions);
Conditions deserializeConditions(const nlohmann::json &j);

} // namespace Simulator
