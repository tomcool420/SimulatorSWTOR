#include "DOT.h"
#include "Target.h"
#include "utility.h"

namespace Simulator {
DamageHits DOT::tick(const TargetPtr &t, const Second &time, bool extraTick) {
    if (!extraTick) {
        _lastTickTime = time;
        ++_TickCount;
    }
    CHECK(getSource());
    auto fs = getAllFinalStats(_ability, getSource(), t);
    auto hits = getHits(_ability, fs, t);
    if (_DoubleTickChance && getRandomValue(0.0, 1.0) < *_DoubleTickChance) {
        auto newHits = getHits(_ability, fs, t);
        hits.insert(hits.end(), newHits.begin(), newHits.end());
    }
    return hits;
}

Debuff *DOT::clone() const { return new DOT(*this); }

DebuffEvents DOT::resolveEventsUpToTime(const Second &time, const TargetPtr &t) {
    DebuffEvents ret;
    while ((getStartTime() + (_TickCount + !_hasInitialTick) * _tickRate <= time + Second(1e-7)) &&
           _nticks != _TickCount) {
        auto expectedTime = getStartTime() + (_TickCount + !_hasInitialTick) * _tickRate;
        ret.push_back({DebuffEventType::Tick, expectedTime, tick(t, time)});
    }
    if (getEndTime() <= time + Second(1e-7))
        ret.push_back({DebuffEventType::Remove});
    return ret;
}

} // namespace Simulator
