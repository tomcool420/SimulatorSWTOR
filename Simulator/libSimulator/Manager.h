#pragma once
#include "Ability.h"
#include "Target.h"
#include <map>

namespace Simulator {

class Manager {
  public:
    void addTarget(const TargetPtr &target);
    const TargetPtr &getTarget(const TargetId &pid) const;
    void runEventsUntilTime(const Second &time);

  private:
    std::map<TargetId, TargetPtr> _targets;
};
} // namespace Simulator
