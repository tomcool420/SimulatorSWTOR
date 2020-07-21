#include "DOT.h"
#include "Target.h"
#include "utility.h"

namespace Simulator {
DamageHits DOT::tick(const TargetPtr &t, const Second &time) {
    _lastTickTime = time;
    ++_tickCount;
    CHECK(getSource());
    auto fs = getFinalStats(_ability, getSource(), t);
    auto hits = getHits(_ability, fs, t);
    if(_doubleTickChance && getRandomValue(0.0, 1.0)<*_doubleTickChance){
        auto newHits = getHits(_ability, fs, t);
        hits.insert(hits.end(), newHits.begin(), newHits.end());
    }
    return hits;
}

Debuff *DOT::clone() const { return new DOT(*this); }

DebuffEvents DOT::resolveEventsUpToTime(const Second & time, const TargetPtr &t){
    DebuffEvents ret;
    while(_lastTickTime+_tickRate<time){
        ret.push_back({DebuffEventType::Tick,_lastTickTime+_tickRate,tick(t,time)});
    }
    if(_nticks==_tickCount){
        ret.push_back({DebuffEventType::Remove});
    }
    return ret;
}

} // namespace Simulator
