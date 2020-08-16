#pragma once

#include "Shadow.h"

namespace Simulator {
class Infiltration : public Shadow {
  public:
    std::vector<BuffPtr> getStaticBuffs() override;
    SIMULATOR_SET_MACRO(AwakenedFlame, bool, false);

  protected:
    [[nodiscard]] AbilityPtr getAbilityInternal(AbilityId id) override;
};
} // namespace Simulator