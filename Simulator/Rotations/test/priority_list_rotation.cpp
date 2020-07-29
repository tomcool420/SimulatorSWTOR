#include "../../Classes/DirtyFighting.h"
#include "../../Classes/detail/shared.h"
#include "../../Rotations/Condition.h"
#include "../../Rotations/PriorityList.h"
#include "../../libSimulator/Rotation.h"
#include "../../libSimulator/Target.h"
#include "../../libSimulator/detail/log.h"
#include "../PriorityListRotation.h"
#include <Simulator/libSimulator/AbilityBuff.h>
#include <Simulator/libSimulator/detail/names.h>
#include <gtest/gtest.h>

using namespace Simulator;
namespace {
struct TestData {
    TargetPtr source;
    TargetPtr target;
    std::shared_ptr<DirtyFighting> df;
};
TestData getTestData(double alacrity = 2331, double crit = 2095) {
    detail::LogDisabler d;
    RawStats rs;
    rs.master = Mastery{12138};
    rs.power = Power{10049};
    rs.accuracyRating = AccuracyRating{1592};
    rs.criticalRating = CriticalRating{crit};
    rs.alacrityRating = AlacrityRating{alacrity};
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {1376.0 * 0.3, 2556.0 * 0.3};
    rs.forceTechPower = FTPower{7008};
    rs.hp = HealthPoints(6.5e6);
    TestData ret;
    ret.source = Target::New(rs);
    auto ab = std::make_unique<AmplifierBuff>();
    ab->setPeriodicIntensityBonus(0.2068);
    ret.source->addBuff(std::move(ab), Second(0.0));
    ret.target = Target::New(rs);
    ret.df = std::make_shared<DirtyFighting>();
    return ret;
}
} // namespace

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
        int totalHits = abl.hitCount + abl.missCount + abl.critCount;
        SIM_INFO("[{:<35} {:>19}]: norm min: {}-{}, crit {}-{}", detail::getAbilityName(abl.id), abl.id,
                 abl.normRange.first, abl.normRange.second, abl.critRange.first, abl.critRange.second);
    }
}
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
struct stats {
    Second mean;
    double stddev;
    Second min;
    Second max;
};
stats getStdDev(const std::vector<Second> &times) {
    std::sort(times.begin(), times.end());
    auto totalTime = std::accumulate(times.begin(), times.end(), Second(0.0));
    auto mean = totalTime / static_cast<double>(times.size());
    std::vector<double> diffsSq(times.size());
    std::transform(times.begin(), times.end(), diffsSq.begin(), [=](const Second &a) -> double {
        auto v = (a - mean).getValue();
        return v * v;
    });
    auto variance = std::accumulate(diffsSq.begin(), diffsSq.end(), 0.0) / times.size();
    auto stddev = std::sqrt(variance);
    return {mean, stddev, times.front(), times.back()};
}
TEST(FullRotation, WoundingShots2) {
    auto lambda = [](bool ef, bool ll, bool ew, bool shattered, double alacrity = 2331.0, double crit = 2095) {
        auto &&[s, t, c] = getTestData();
        auto d = new detail::LogDisabler;
        auto p = std::make_shared<PriorityList>();
        p->addAbility(gunslinger_smugglers_luck, {getCooldownFinishedCondition(gunslinger_smugglers_luck)});
        p->addAbility(gunslinger_hunker_down, {getCooldownFinishedCondition(gunslinger_hunker_down)});
        p->addAbility(gunslinger_illegal_mods, {getCooldownFinishedCondition(gunslinger_illegal_mods)});
        auto baseRotation = std::make_shared<StaticRotation>();
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(gunslinger_vital_shot);
        baseRotation->addAbility(dirty_fighting_shrap_bomb);
        baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);
        p->addPriorityList(baseRotation, {});
        addBuffs(s, c->getStaticBuffs(), Second(0.0));
        s->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
        s->addBuff(std::make_unique<RelicProcBuff>(relic_mastery_surge, Mastery{2892}, Power{0}, CriticalRating{0.0}),
                   Second(0.0));
        s->addBuff(std::make_unique<RelicProcBuff>(relic_power_surge, Mastery{0.0}, Power{2892}, CriticalRating{0.0}),
                   Second(0.0));
        c->setExploitedWeakness(ew);
        c->setLayLowPassive(ll);
        c->setEstablishedFoothold(ef);
        if (shattered) {
            t->addDebuff(detail::getGenericDebuff(debuff_shattered), s, Second(0.0));
        }
        PriorityListRotation rot(s);
        rot.setNextFreeGCD(Second(0.0));
        rot.setTarget(t);
        rot.setClass(c);
        rot.setPriorityList(p);
        rot.setMinTimeAfterInstant(Second(0.1));
        rot.setDelayAfterChanneled(Second(0.15));
        rot.doRotation();
        delete d;
        return t->getEvents();
    };
    int iterations = 20;
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(false, true, true, false);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death (no Established Foothold): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, false, true, false);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death (no Lay Low): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, false);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death: {} seconds", totalTime.getValue() / iterations);
    }

    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, false, false);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death (no exploited weakness): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death (Shattered): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death (Shattered): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true, 1214, 3206);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death Baseline 1.4 (Shattered): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true, 3208, 1232);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death Baseline 1.3 (Shattered): {} seconds", totalTime.getValue() / iterations);
    }
    {
        auto &&[s, t, c] = getTestData();
        auto p = std::make_shared<PriorityList>();
        p->addAbility(gunslinger_smugglers_luck, {getCooldownFinishedCondition(gunslinger_smugglers_luck)});
        p->addAbility(gunslinger_hunker_down, {getCooldownFinishedCondition(gunslinger_hunker_down)});
        p->addAbility(gunslinger_illegal_mods, {getCooldownFinishedCondition(gunslinger_illegal_mods)});
        auto baseRotation = std::make_shared<StaticRotation>();
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(gunslinger_vital_shot);
        baseRotation->addAbility(dirty_fighting_shrap_bomb);
        baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);
        p->addPriorityList(baseRotation, {});
        addBuffs(s, c->getStaticBuffs(), Second(0.0));
        t->setLogEvents(false);
        s->addBuff(std::make_unique<RelicProcBuff>(relic_mastery_surge, Mastery{2892}, Power{0}, CriticalRating{0.0}),
                   Second(0.0));
        s->addBuff(std::make_unique<RelicProcBuff>(relic_power_surge, Mastery{0.0}, Power{2892}, CriticalRating{0.0}),
                   Second(0.0));
        s->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
        c->setExploitedWeakness(true);
        c->setLayLowPassive(true);
        c->setEstablishedFoothold(true);
        auto vs = c->getAbility(dirty_fighting_dirty_blast);
        auto afs = getAllFinalStats(*vs, s, t);
        auto drs = calculateDamageRange(*vs, afs);
        for (auto dr : drs)
            SIM_INFO("vs range  {}-{}", dr.dmg.first, dr.dmg.second);
        auto hits = getHits(*vs, afs, t);
        for (auto hit : hits)
            SIM_INFO("vs hit  {}", hit.dmg);
        t->addDebuff(detail::getGenericDebuff(debuff_shattered), s, Second(0.0));

        PriorityListRotation rot(s);
        rot.setNextFreeGCD(Second(0.0));
        rot.setTarget(t);
        rot.setClass(c);
        rot.setPriorityList(p);

        auto d = new detail::LogDisabler;
        rot.doRotation();
        delete d;
        // SIM_INFO("Target died after {} seconds", t->getDeathTime()->getValue());
        auto &&events = t->getEvents();
        auto tt = getLastDamageEvent(t->getEvents()) - getFirstDamageEvent(t->getEvents());
        SIM_INFO("It took {} seconds to kill target", tt.getValue());
        logParseInformation(events, tt);
    }
}

TEST(FullRotation, WoundingShots3) {
    auto lambda = [](bool ef, bool ll, bool ew, bool shattered, double alacrity = 2331.0, double crit = 2095) {
        auto &&[s, t, c] = getTestData();
        auto d = new detail::LogDisabler;
        auto p = std::make_shared<PriorityList>();
        p->addAbility(gunslinger_smugglers_luck, {getCooldownFinishedCondition(gunslinger_smugglers_luck)});
        p->addAbility(gunslinger_hunker_down, {getCooldownFinishedCondition(gunslinger_hunker_down)});
        p->addAbility(gunslinger_illegal_mods, {getCooldownFinishedCondition(gunslinger_illegal_mods)});
        auto baseRotation = std::make_shared<StaticRotation>();
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(gunslinger_vital_shot);
        baseRotation->addAbility(dirty_fighting_shrap_bomb);
        baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);

        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);

        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);

        baseRotation->addAbility(gunslinger_vital_shot);
        baseRotation->addAbility(dirty_fighting_shrap_bomb);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);

        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);

        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_dirty_blast);
        baseRotation->addAbility(dirty_fighting_wounding_shots);

        p->addPriorityList(baseRotation, {});
        addBuffs(s, c->getStaticBuffs(), Second(0.0));
        s->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
        s->addBuff(std::make_unique<RelicProcBuff>(relic_mastery_surge, Mastery{2892}, Power{0}, CriticalRating{0.0}),
                   Second(0.0));
        s->addBuff(std::make_unique<RelicProcBuff>(relic_power_surge, Mastery{0.0}, Power{2892}, CriticalRating{0.0}),
                   Second(0.0));
        c->setExploitedWeakness(ew);
        c->setLayLowPassive(ll);
        c->setEstablishedFoothold(ef);
        if (shattered) {
            t->addDebuff(detail::getGenericDebuff(debuff_shattered), s, Second(0.0));
        }
        PriorityListRotation rot(s);
        rot.setNextFreeGCD(Second(0.0));
        rot.setTarget(t);
        rot.setClass(c);
        rot.setPriorityList(p);
        rot.setMinTimeAfterInstant(Second(0.1));
        rot.setDelayAfterChanneled(Second(0.15));
        rot.doRotation();
        delete d;
        return t->getEvents();
    };
    int iterations = 20;
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(false, true, true, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (no Established Foothold): {} seconds (stddev = {})", mean.getValue(), stddev);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, false, true, false);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death (no Lay Low): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, false);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death: {} seconds", totalTime.getValue() / iterations);
    }

    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, false, false);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death (no exploited weakness): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death (Shattered): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true, 1214, 3206);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death Baseline 1.4 (Shattered): {} seconds", totalTime.getValue() / iterations);
    }
    {
        Second totalTime{0.0};
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true, 3208, 1232);
            totalTime += getLastDamageEvent(events) - getFirstDamageEvent(events);
        }
        SIM_INFO("Mean Time to Death Baseline 1.3 (Shattered): {} seconds", totalTime.getValue() / iterations);
    }

    auto &&events = lambda(true, true, true, true);
    auto duration = getLastDamageEvent(events) - getFirstDamageEvent(events);
    logParseInformation(events, duration);
}
