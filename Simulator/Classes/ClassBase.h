#pragma once
#include <Simulator/SimulatorBase/Class.h>

namespace Simulator {
class ClassBase : public Class {
  public:
    std::vector<BuffPtr> getStaticBuffs() override;
    void loadOptions(const nlohmann::json &j) override;
    nlohmann::json serialize() override;

    SIMULATOR_SET_MACRO(Relic1, AbilityId, 0);
    SIMULATOR_SET_MACRO(Relic2, AbilityId, 0);
    SIMULATOR_SET_MACRO(ClassBuffs, bool, false);
};
} // namespace Simulator