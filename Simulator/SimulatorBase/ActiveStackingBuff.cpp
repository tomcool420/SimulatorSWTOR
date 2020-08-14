#include "ActiveStackingBuff.h"

namespace Simulator {
void ActiveStackingBuff::activate(const Second &time) {
    auto charges = std::min(getCurrentStacks(), getStacksOnActivation());
    setCurrentStacks(std::min(getMaxStacks(), charges + getCurrentStacks()), time);
    if (getConsumesChargesOnActivation())
        setCurrentCharges(getCurrentCharges() - charges);
    if (!_nextChargeGain && getChargeCooldown())
        _nextChargeGain = time + getChargeCooldown().value();
}
std::optional<Second> ActiveStackingBuff::getNextEventTime() const {
    auto nbe = Buff::getNextEventTime();
    if (_nextChargeGain && nbe) {
        return std::min(*_nextChargeGain, *nbe);
    }
    if (_nextChargeGain)
        return _nextChargeGain;
    return nbe;
}
[[nodiscard]] DebuffEvents ActiveStackingBuff::resolveEventsUpToTime(const Second &time, const TargetPtr &t) {
    DebuffEvents ret;
    if (_nextChargeGain && time > _nextChargeGain.value()) {
        setCurrentCharges(getCurrentCharges() + 1);
        if (getCurrentCharges() == getMaxCharges()) {
            _nextChargeGain = std::nullopt;
        } else if (auto cd = getChargeCooldown()) {
            _nextChargeGain = _nextChargeGain.value() + cd.value();
        }
        ret.push_back({DebuffEventType::Internal});
    }
    auto pe = Buff::resolveEventsUpToTime(time, t);
    ret.insert(ret.end(), pe.begin(), pe.end());
    return ret;
}

} // namespace Simulator