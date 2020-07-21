#pragma once
#include "types.h"
#include "utility.h"

namespace Simulator {
class Rotation {
  public:
    Rotation(TargetPtr source) : _source(source) {}
    virtual std::optional<Second> getNextEventTime();

  private:
    TargetPtr _source;
    TargetPtrs _targets;
}
} // namespace Simulator