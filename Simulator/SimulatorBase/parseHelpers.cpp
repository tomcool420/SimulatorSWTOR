#include "parseHelpers.h"
#include "detail/log.h"
#include "detail/names.h"
namespace Simulator {
Second getFirstDamageEvent(const Target::TargetEvents &events) {
    for (auto &&e : events) {
        if (e.type == Target::TargetEventType::Damage)
            return e.time;
    }
    CHECK(false);
    return Second(0.0);
}
Second getLastDamageEvent(const Target::TargetEvents &events) {
    for (auto e = events.rbegin(); e != events.rend(); ++e) {
        if (e->type == Target::TargetEventType::Damage)
            return e->time;
    }
    CHECK(false);
    return Second(0.0);
}

struct AbilityLogInformation {
    AbilityId id;
    double totalDamage{0};
    int hitCount{0};
    int critCount{0};
    int missCount{0};
    std::pair<int, int> critRange{100000, 0};
    std::pair<int, int> normRange{100000, 0};
};

std::map<AbilityId, AbilityLogInformation> getEventInformation(const Target::TargetEvents &events) {
    std::map<AbilityId, AbilityLogInformation> ret;
    for (auto &&event : events) {
        if (event.type != Target::TargetEventType::Damage)
            continue;
        for (auto &&hit : *event.damage) {
            auto &&info = ret[hit.id];
            info.id = hit.id;
            info.totalDamage += hit.dmg;
            if (hit.crit) {
                info.critCount += 1;
                info.critRange.second = std::max<int>(static_cast<int>(std::round(hit.dmg)), info.critRange.second);
                info.critRange.first = std::min<int>(static_cast<int>(std::round(hit.dmg)), info.critRange.first);
            } else if (hit.miss)
                info.missCount += 1;
            else {
                info.hitCount += 1;
                info.normRange.second = std::max<int>(static_cast<int>(std::round(hit.dmg)), info.normRange.second);
                info.normRange.first = std::min<int>(static_cast<int>(std::round(hit.dmg)), info.normRange.first);
            }
        }
    }
    return ret;
}

void logParseInformation(const Target::TargetEvents &events, Second duration) {
    auto abilities = getEventInformation(events);
    std::vector<AbilityLogInformation> informations;
    for (auto &&abl : abilities) {
        informations.push_back(abl.second);
    }
    std::sort(
        informations.begin(), informations.end(),
        [](const AbilityLogInformation &a, const AbilityLogInformation &b) { return a.totalDamage > b.totalDamage; });
    auto totalDamage =
        std::accumulate(informations.begin(), informations.end(), 0.0,
                        [](const double &s, const AbilityLogInformation &b) { return s + b.totalDamage; });
    auto tt = getLastDamageEvent(events) - getFirstDamageEvent(events);
    SIM_INFO("It took {} seconds to kill target", tt.getValue());
    for (auto &&abl : informations) {
        int totalHits = abl.hitCount + abl.missCount + abl.critCount;
        SIM_INFO("[{:<35} {:>19}]: Hits: {:>4}, Normal Hits {:>4} ({:>02.2f}%), Crits: {:>4} ({:6.2f}%), Misses: {:>4} "
                 "({:>5.2f}%), DPS: {:>7}, Percentage: {}",
                 detail::getAbilityName(abl.id), abl.id, totalHits, abl.hitCount,
                 (double)abl.hitCount / totalHits * 100.0, abl.critCount, (double)abl.critCount / totalHits * 100.0,
                 abl.missCount, (double)abl.missCount / totalHits * 100.0, abl.totalDamage / duration.getValue(),
                 abl.totalDamage / totalDamage * 100.0);
    }
    for (auto &&abl : informations) {
        // int totalHits = abl.hitCount + abl.missCount + abl.critCount;
        SIM_INFO("[{:<35} {:>19}]: norm min: {}-{}, crit {}-{}", detail::getAbilityName(abl.id), abl.id,
                 abl.normRange.first, abl.normRange.second, abl.critRange.first, abl.critRange.second);
    }
}
} // namespace Simulator