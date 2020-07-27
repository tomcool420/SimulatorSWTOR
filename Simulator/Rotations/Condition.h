#pragma once
#include "Simulator/libSimulator/Target.h"
#include "Simulator/libSimulator/types.h"
namespace Simulator {
using Condition = std::function<bool(const TargetPtr &, const TargetPtr &, const Second &, const Second &)>;
using Conditions = std::vector<Condition>;
Condition getCooldownFinishedCondition(AbilityId id);

} // namespace Simulator