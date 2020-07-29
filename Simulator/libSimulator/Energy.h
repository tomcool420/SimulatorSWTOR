#pragma once
#include "detail/units.h"
#include "types.h"
#include "utility.h"
#include <optional>
#include <vector>
namespace Simulator {
struct EnergyReturn {
    double gainedEnergy;
};
class Energy {
  public:
    void addEnergyRegenThreshold(int threshold, double baseEnergyRegen);
    EnergyReturn resolveEventsUpToTime(const Second &time, const TargetPtr &);
    [[nodiscard]] std::optional<Second> getNextEventTime() const;
    void spendEnergy(int energy, const Second &time);
    EnergyReturn addEnergy(int energy, const Second &time);
    void addEnergyThreshold(int t, double amount);
    SIMULATOR_SET_MACRO(MaxEnergy, int, 100);
    SIMULATOR_SET_MACRO(CurrentEnergy, double, 100.0);
    SIMULATOR_SET_MACRO(EnergyTickRate, Second, Second(1.0));
    SIMULATOR_SET_MACRO(Verbose, bool, false);

  private:
    std::vector<std::pair<int, double>> _energyThresholds;
    std::optional<Second> _nextTickTime;
};
} // namespace Simulator