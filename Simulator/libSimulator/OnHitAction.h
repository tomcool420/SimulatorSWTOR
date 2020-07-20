#pragma once
#include "types.h"

namespace Simulator {
class OnHitAction {
  public:
    virtual void doOnHit(const DamageHits &/*hits*/,const TargetPtr &/*source*/, const TargetPtr &/*target*/, const Second &/*time*/){};
    virtual ~OnHitAction() = default;
};
using OnHitActionPtr = std::shared_ptr<OnHitAction>;
using OnHitActionPtrs = std::vector<OnHitActionPtr>;
} // namespace Simulator
