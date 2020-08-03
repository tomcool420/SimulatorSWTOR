#pragma once
#include "Simulator/SimulatorBase/Class.h"
#include "Simulator/SimulatorBase/utility.h"
#include <Simulator/SimulatorBase/Energy.h>
namespace Simulator {

class Gunslinger : public Class {
  public:
    virtual ~Gunslinger() = default;
    EnergyPtr getEnergyModel() override {
        if (!_energy) {
            _energy = std::make_shared<Energy>();
            _energy->setMaxEnergy(110);
            _energy->setCurrentEnergy(110);
            _energy->addEnergyThreshold(66, 5);
            _energy->addEnergyThreshold(22, 3);
            _energy->addEnergyThreshold(0, 2);
            _energy->setEnergyTickRate(Second(1.0));
        }
        return _energy;
    }
    void loadOptions(const nlohmann::json &j);

  protected:
    EnergyPtr _energy{nullptr};
    [[nodiscard]] AbilityPtr getAbilityInternal(AbilityId id) override;
    std::vector<BuffPtr> getStaticBuffs() override;
    SIMULATOR_SET_MACRO(EstablishedFoothold, bool, false);
    SIMULATOR_SET_MACRO(ExploitedWeakness, bool, false);
    SIMULATOR_SET_MACRO(LayLowPassive, bool, true);
};

} // namespace Simulator
