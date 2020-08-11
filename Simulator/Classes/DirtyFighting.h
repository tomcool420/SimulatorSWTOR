#pragma once
#include "Gunslinger.h"

namespace Simulator {
class DirtyFighting : public Gunslinger {
  public:
    std::vector<BuffPtr> getStaticBuffs() override;
    virtual ~DirtyFighting() = default;
    nlohmann::json serialize() override;

  protected:
    AbilityPtr getAbilityInternal(AbilityId id) override;
};
} // namespace Simulator
