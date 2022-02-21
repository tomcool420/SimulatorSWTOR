#pragma once
#include "Gunslinger.h"

namespace Simulator {
class Sharpshooter : public Gunslinger {
  public:
    BuffPtrs getStaticBuffs() override;
    virtual ~Sharpshooter() = default;
    nlohmann::json serialize() override;

  protected:
    AbilityPtr getAbilityInternal(AbilityId id) override;
};
} // namespace Simulator
