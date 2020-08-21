#include "Buff.h"
#include "detail/log.h"
#include "detail/names.h"
#include <mutex>
namespace Simulator {
namespace {
uint64_t _lastIndex = 1000;
std::mutex _mutex;
} // namespace
uint64_t getNextFreeId() {
    std::lock_guard m(_mutex);
    return _lastIndex++;
}
void Buff::setCurrentStacks(int s, const Second &time) {
    s = std::clamp(s, 0, getMaxStacks());
    SIM_INFO("Time : {}, buff: [{} {}] setting stacks to {}", time.getValue(), detail::getAbilityName(getId()), getId(),
             s);
    if (s == 0)
        _stackExpiration = std::nullopt;
    else if (s >= _CurrentStacks)
        _stackExpiration = time + getStackDuration().value();
    _CurrentStacks = s;
}
[[nodiscard]] std::optional<Second> Buff::getNextEventTime() const {
    auto nte = TimedStatusEffect::getNextEventTime();
    if (_stackExpiration && nte && _stackExpiration.value() < nte.value())
        return _stackExpiration;
    if (_stackExpiration)
        return _stackExpiration;
    return nte;
}
DebuffEvents Buff::resolveEventsUpToTime(const Second &time, const TargetPtr &) {
    DebuffEvents ret;
    if (_stackExpiration && time > _stackExpiration) {
        setCurrentStacks(0, time);
        _stackExpiration = std::nullopt;
        ret.push_back({DebuffEventType::Internal});
    }
    if (!isIndeterminate() && time > getEndTime()) {
        ret.push_back({DebuffEventType::Remove});
    }
    return ret;
}
void Buff::adjustEnergyAndCastTime(const Ability &, EnergyCost &, Second &, bool &) {}
} // namespace Simulator
