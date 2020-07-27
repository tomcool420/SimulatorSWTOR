#include "Energy.h"

namespace Simulator {
void Energy::spendEnergy(int energy, const Second &time) {
    CHECK(_CurrentEnergy > energy);
    _CurrentEnergy -= energy;
    if (_energyThresholds.size() && !_nextTickTime)
        _nextTickTime = _EnergyTickRate / 1.0 + time;
}
EnergyReturn Energy::addEnergy(int energy, const Second &) {
    double oldE = _CurrentEnergy;
    _CurrentEnergy = std::min<double>(_CurrentEnergy + energy, _MaxEnergy);
    return {_CurrentEnergy - oldE};
}
std::optional<Second> Energy::getNextEventTime() const { return _nextTickTime; }
EnergyReturn Energy::resolveEventsUpToTime(const Second &time, const TargetPtr &) {
    double oldE = _CurrentEnergy;
    if (_CurrentEnergy < _MaxEnergy && _nextTickTime < time + _tinyTime) {
        for (auto &&t : _energyThresholds) {
            if (_CurrentEnergy >= t.first) {
                // TODO: adjust energy regen for alacrity
                _CurrentEnergy = std::min<double>(_CurrentEnergy + t.second, _MaxEnergy);
                break;
            }
        }
    }
    CHECK(_nextTickTime)
    _nextTickTime.value() += _EnergyTickRate;
    return {_CurrentEnergy - oldE};
}
void Energy::addEnergyThreshold(int t, double amount) {
    _energyThresholds.push_back({t, amount});
    std::sort(_energyThresholds.begin(), _energyThresholds.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });
}

} // namespace Simulator