#pragma once
#include "Gunslinger.h"

namespace Simulator {
class DirtyFighting : public Gunslinger {
public:
    std::vector<BuffPtr> getStaticBuffs() override;
    virtual ~DirtyFighting() = default;
  protected:
    AbilityPtr getAbilityInternal(AbilityId id) override;
    

};
} // namespace Simulator
