#pragma once
#include "../libSimulator/Class.h"
#include "../libSimulator/utility.h"
namespace Simulator {

class Gunslinger : public Class {
  public:
    virtual ~Gunslinger() = default;
  protected:
    [[nodiscard]] AbilityPtr getAbilityInternal(AbilityId id) override;
    std::vector<BuffPtr> getStaticBuffs() override;
    SIMULATOR_SET_MACRO(EstablishedFoothold, bool, false);
    SIMULATOR_SET_MACRO(ExploitedWeakness, bool, false);
    SIMULATOR_SET_MACRO(LayLowPassive, bool, true);

};

} // namespace Simulator
