#pragma once
#include <Simulator/SimulatorBase/Class.h>

namespace Simulator {
using TalentList = std::array<uint8_t, 8>;
class ClassBase : public Class {
  public:
    std::vector<BuffPtr> getStaticBuffs() override;
    void loadOptions(const nlohmann::json &j) override;
    nlohmann::json serialize() override;
    void setTalents(TalentList talents) { _talents = std::move(talents); }
    TalentList &getTalents() { return _talents; }

  private:
    TalentList _talents{1, 1, 1, 1, 1, 1, 1, 1};
    SIMULATOR_SET_MACRO(Relic1, AbilityId, 0);
    SIMULATOR_SET_MACRO(Relic2, AbilityId, 0);
    SIMULATOR_SET_MACRO(ClassBuffs, bool, false);
};
} // namespace Simulator