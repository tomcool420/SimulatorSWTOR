#pragma once
#include "Condition.h"
#include "Simulator/libSimulator/types.h"
#include <nlohmann/json.hpp>
#include <variant>

namespace Simulator {

using RotationalReturn = std::variant<AbilityId, Second>;
class RotationalPriorityList;
using RotationalPriorityListPtr = std::shared_ptr<RotationalPriorityList>;
class RotationalPriorityList {
  public:
    virtual void log(std::ostream &stream, int indent) const = 0;
    [[nodiscard]] virtual RotationalReturn getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                          const Second &nextInstant, const Second &nextGCD) = 0;
    nlohmann::json virtual serialize() const { return nlohmann::json(); }
    static RotationalPriorityListPtr deserialize(const nlohmann::json &j);
    virtual ~RotationalPriorityList() = default;
};

class PriorityList : public RotationalPriorityList {
  public:
    [[nodiscard]] RotationalReturn getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                  const Second &nextInstant, const Second &nextGCD) override;
    void addAbility(AbilityId aId, Conditions &&conditions) { _priorites.push_back({aId, std::move(conditions)}); }
    void addPriorityList(RotationalPriorityListPtr rpl, Conditions &&conditions) {
        _priorites.push_back({rpl, std::move(conditions)});
    }
    void log(std::ostream &stream, int indent = 0) const override;
    nlohmann::json serialize() const override;
    static std::shared_ptr<PriorityList> deserialize(const nlohmann::json &j);

    virtual ~PriorityList() = default;

  private:
    struct Priority {
        std::variant<AbilityId, RotationalPriorityListPtr> action;
        Conditions conditions;
    };
    std::vector<Priority> _priorites;
};

constexpr char key_ability[] = "ability";
constexpr char key_delay[] = "delay";
constexpr char key_other[] = "other";
constexpr char key_rotation[] = "rotation";
constexpr char key_type[] = "type";
constexpr char key_conditions[] = "conditions";
constexpr char key_priority[] = "priority";
} // namespace Simulator