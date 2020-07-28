#pragma once
#include "Condition.h"
#include "Simulator/libSimulator/types.h"
#include <variant>
namespace Simulator {

using RotationalReturn = std::variant<AbilityId, Second>;
class RotationalPriorityList {
  public:
    virtual void log(std::ostream &stream, int indent) const = 0;
    [[nodiscard]] virtual RotationalReturn getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                          const Second &nextInstant, const Second &nextGCD) = 0;
    virtual ~RotationalPriorityList() = default;
};
using RotationalPriorityListPtr = std::shared_ptr<RotationalPriorityList>;

class PriorityList : public RotationalPriorityList {
  public:
    struct Priority {
        std::variant<AbilityId, RotationalPriorityListPtr> action;
        Conditions conditions;
    };
    void addAbility(AbilityId aId, Conditions &&conditions) { _priorites.push_back({aId, std::move(conditions)}); }
    void addOtherId(RotationalPriorityListPtr rpl, Conditions &&conditions) {
        _priorites.push_back({rpl, std::move(conditions)});
    }
    [[nodiscard]] RotationalReturn getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                  const Second &nextInstant, const Second &nextGCD) override;
    void log(std::ostream &stream, int indent = 0) const override;
    virtual ~PriorityList() = default;

  private:
    std::vector<Priority> _priorites;
};
} // namespace Simulator