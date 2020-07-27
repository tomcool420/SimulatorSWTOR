#pragma once
#include "Condition.h"
#include "Simulator/libSimulator/types.h"
#include <variant>
namespace Simulator {

using RotationalReturn = AbilityId;
class RotationalPriorityList {
  public:
    virtual void log(std::iostream &stream, int indent) = 0;
    [[nodiscard]] virtual RotationalReturn getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                          const Second &nextInstant, const Second &nextGCD) = 0;
};
using RotationalPriorityListPtr = std::shared_ptr<RotationalPriorityList>;

class PriorityList : RotationalPriorityList {
  public:
    struct Priority {
        std::optional<AbilityId> aId;
        std::optional<RotationalPriorityListPtr> rpl;
        Conditions conditions;
    };
    void addAbility(AbilityId aId, Conditions &&conditions) {
        Priority p;
        p.aId = aId;
        p.conditions = std::move(conditions);
        _priorites.push_back(std::move(p));
    }
    void addOtherId(RotationalPriorityListPtr rpl, Conditions &&conditions) {
        Priority p;
        p.rpl = rpl;
        p.conditions = std::move(conditions);
        _priorites.push_back(std::move(p));
    }
    [[nodiscard]] RotationalReturn getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                  const Second &nextInstant, const Second &nextGCD) override {
        for (auto &&p : _priorites) {
            bool satisfiedConditions = true;
            for (auto &&c : p.conditions) {
                if (!c(source, target, nextInstant, nextGCD)) {
                    satisfiedConditions = false;
                    break;
                }
            }
            if (satisfiedConditions) {
                if (p.aId) {
                    return p.aId.value();
                } else if (p.rpl) {
                    return 0;
                    // return p.rpl->getNextAbility(source, target, nextInstant, nextGCD);
                } else {
                    CHECK(false, "This is a slot with no ability or other rotation!!!");
                }
            }
        }
        CHECK(false, "No Abilities left in priority list. it is ill formed");
        return 0;
    }
    void log(std::iostream &stream, int indent = 0) {
        stream << fmt::format("{<:{}} Priority list with {} items", "", indent, _priorites.size());
    }

  private:
    std::vector<Priority> _priorites;
};
} // namespace Simulator