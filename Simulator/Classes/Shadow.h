#pragma once
#include "ClassBase.h"

namespace Simulator {
class Shadow : public ClassBase {
  public:
    EnergyPtr getEnergyModel() override { return nullptr; }
    void loadOptions(const nlohmann::json &j) override {}
    nlohmann::json serialize() override { return {}; }
    std::vector<BuffPtr> getStaticBuffs() override;

    SIMULATOR_SET_MACRO(ForceHarmonics, bool, true);
    SIMULATOR_SET_MACRO(Fade, bool, true);

  protected:
    [[nodiscard]] AbilityPtr getAbilityInternal(AbilityId id) override;
};
} // namespace Simulator