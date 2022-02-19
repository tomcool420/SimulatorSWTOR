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
    SIMULATOR_SET_MACRO(Talents, std::array<uint8_t, 8>, {1, 1, 1, 1, 1, 1, 1, 1});
};
} // namespace Simulator