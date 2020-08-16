#pragma once
#include "ClassBase.h"
#include <Simulator/SimulatorBase/Energy.h>
#include <Simulator/SimulatorBase/utility.h>

namespace Simulator {
class Sentinel : public ClassBase {
  public:
    virtual ~Sentinel() = default;
    void loadOptions(const nlohmann::json &j) override;
    nlohmann::json serialize() override;
    [[nodiscard]] AbilityPtr getAbilityInternal(AbilityId id) override;
    std::vector<BuffPtr> getStaticBuffs() override;
};

} // namespace Simulator