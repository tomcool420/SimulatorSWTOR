#include "Simulator/SimulatorBase/Rotation.h"
#include "Simulator/SimulatorBase/Target.h"
#include "Simulator/SimulatorBase/detail/log.h"
#include "../../Classes/DirtyFighting.h"
#include "../../Classes/detail/shared.h"
#include "../../Rotations/Condition.h"
#include "../../Rotations/PriorityList.h"
#include "../PriorityListRotation.h"
#include <Simulator/SimulatorBase/AbilityBuff.h>
#include <Simulator/SimulatorBase/detail/names.h>
#include <Simulator/SimulatorBase/parseHelpers.h>
#include <fstream>
#include <gtest/gtest.h>
#include <tbb/parallel_for.h>

using namespace Simulator;
namespace {
struct TestData {
    TargetPtr source;
    TargetPtr target;
    std::shared_ptr<DirtyFighting> df;
};
TestData getTestData(double alacrity = 2331, double crit = 2095, bool /*unused*/=true, bool masteryBonus = 0) {
    detail::LogDisabler d;
    RawStats rs;
    rs.master = Mastery{12138} + Mastery{masteryBonus};
    rs.power = Power{10049}; // - Power{500};
    rs.accuracyRating = AccuracyRating{1592};
    rs.criticalRating = CriticalRating{crit};
    rs.alacrityRating = AlacrityRating{alacrity};
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {1376.0 * 0.3, 2556.0 * 0.3};
    rs.forceTechPower = FTPower{7008};
    rs.hp = HealthPoints(6.5e6);
    TestData ret;
    ret.source = Target::New(rs);
    ret.target = Target::New(rs);
    ret.df = std::make_shared<DirtyFighting>();
    return ret;
}
} // namespace

struct stats {
    Second mean;
    double stddev;
    Second min;
    Second max;
};
stats getStdDev(const std::vector<Second> &times) {
    Second max{0.0};
    Second min{1e6};
    for (auto &t : times) {
        min = std::min(t, min);
        max = std::max(t, max);
    }
    auto totalTime = std::accumulate(times.begin(), times.end(), Second(0.0));
    auto mean = totalTime / static_cast<double>(times.size());
    std::vector<double> diffsSq(times.size());
    std::transform(times.begin(), times.end(), diffsSq.begin(), [=](const Second &a) -> double {
        auto v = (a - mean).getValue();
        return v * v;
    });
    auto variance = std::accumulate(diffsSq.begin(), diffsSq.end(), 0.0) / times.size();
    auto stddev = std::sqrt(variance);
    return {mean, stddev, min, max};
}
TEST(FullRotation, DISABLED_WoundingShots2) {
    auto lambda = [](bool ef, bool ll, bool ew, bool shattered, double alacrity = 2331.0, double crit = 2095) {
        auto &&[s, t, c] = getTestData(alacrity, crit);
        auto d = new detail::LogDisabler;
        auto p = std::make_shared<PriorityList>();
        p->addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
        p->addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
        p->addAbility(gunslinger_illegal_mods, getCooldownFinishedCondition(gunslinger_illegal_mods));
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
        rot.setMinTimeAfterInstant(Second(0.0));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.doRotation();
        delete d;
        return t->getEvents();
    };
    int iterations = 500;
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(false, true, true, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (no Established Foothold): {} seconds (stddev = {}, min = {}, max = {})",
                 mean.getValue(), stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, false, true, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (no Lay Low): {} seconds (stddev = {}, min = {}, max = {})", mean.getValue(),
                 stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, false, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (no Exploited Weakness): {} seconds (stddev = {}, min = {}, max = {})",
                 mean.getValue(), stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death : {} seconds (stddev = {}, min = {}, max = {})", mean.getValue(), stddev, minV,
                 maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (Shattered): {} seconds (stddev = {}, min = {}, max = {})", mean.getValue(),
                 stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true, 1214, 3206);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death Baseline 1.4 (Shattered): {} seconds (stddev = {}, min = {}, max = {})",
                 mean.getValue(), stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true, 3208, 1232);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death Baseline 1.3 (Shattered): {} seconds (stddev = {}, min = {}, max = {})",
                 mean.getValue(), stddev, minV, maxV);
    }
    auto &&events = lambda(true, true, true, true);
    auto duration = getLastDamageEvent(events) - getFirstDamageEvent(events);
    logParseInformation(events, duration);
}

TEST(FullRotation, WoundingShots3) {
    bool disableLog = true;
    bool wireInEnergy = false;
    auto lambda = [&](bool ef, bool ll, bool ew, bool shattered, double alacrity = 2331.0, double crit = 2095) {
        auto &&[s, t, c] = getTestData(alacrity, crit);
        detail::LogDisabler *d = nullptr;
        if (disableLog)
            d = new detail::LogDisabler;
        if (wireInEnergy)
            s->setEnergyModel(c->getEnergyModel());
        auto p = std::make_shared<PriorityList>();
        p->addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
        p->addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
        p->addAbility(gunslinger_illegal_mods, getCooldownFinishedCondition(gunslinger_illegal_mods));
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
        rot.setMinTimeAfterInstant(Second(0.0));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.doRotation();
        if (d)
            delete d;
        return t->getEvents();
    };
    int iterations = 1;
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(false, true, true, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (no Established Foothold): {} seconds (stddev = {}, min = {}, max = {})",
                 mean.getValue(), stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, false, true, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (no Lay Low): {} seconds (stddev = {}, min = {}, max = {})", mean.getValue(),
                 stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, false, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (no Exploited Weakness): {} seconds (stddev = {}, min = {}, max = {})",
                 mean.getValue(), stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, false);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death : {} seconds (stddev = {}, min = {}, max = {})", mean.getValue(), stddev, minV,
                 maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death (Shattered): {} seconds (stddev = {}, min = {}, max = {})", mean.getValue(),
                 stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true, 1214, 3206);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death Baseline 1.4 (Shattered): {} seconds (stddev = {}, min = {}, max = {})",
                 mean.getValue(), stddev, minV, maxV);
    }
    {
        std::vector<Second> times;
        for (int ii = 0; ii < iterations; ++ii) {
            auto &&events = lambda(true, true, true, true, 3208, 1232);
            auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
            times.push_back(time);
        }
        auto &&[mean, stddev, minV, maxV] = getStdDev(times);
        SIM_INFO("Mean Time to Death Baseline 1.3 (Shattered): {} seconds (stddev = {}, min = {}, max = {})",
                 mean.getValue(), stddev, minV, maxV);
    }
    wireInEnergy = true;
    ASSERT_NO_THROW({
        auto events = lambda(true, true, true, true);
        auto duration = getLastDamageEvent(events) - getFirstDamageEvent(events);
        logParseInformation(events, duration);
    });
}

TEST(FullRotation, DISABLED_WoundingShots2AlacrityRangeCritRelic) {
    auto d = new detail::LogDisabler;

    auto lambda = [](bool ef, bool ll, bool ew, bool shattered, double alacrity = 2331.0, double crit = 2095,
                     double mastery = 0, bool critRelic = false) {
        auto &&[s, t, c] = getTestData(alacrity, crit, true, mastery);
        auto p = std::make_shared<PriorityList>();
        p->addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
        p->addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
        p->addAbility(gunslinger_illegal_mods, getCooldownFinishedCondition(gunslinger_illegal_mods));

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

        // auto p = std::make_shared<PriorityList>();
        // p->addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
        // p->addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
        // p->addAbility(gunslinger_illegal_mods, getCooldownFinishedCondition(gunslinger_illegal_mods));
        // auto baseRotation = std::make_shared<StaticRotation>();
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(gunslinger_vital_shot);
        // baseRotation->addAbility(dirty_fighting_shrap_bomb);
        // baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
        // baseRotation->addAbility(dirty_fighting_wounding_shots);

        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_wounding_shots);

        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
        // baseRotation->addAbility(dirty_fighting_wounding_shots);

        // baseRotation->addAbility(gunslinger_vital_shot);
        // baseRotation->addAbility(dirty_fighting_shrap_bomb);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_wounding_shots);

        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
        // baseRotation->addAbility(dirty_fighting_wounding_shots);

        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_dirty_blast);
        // baseRotation->addAbility(dirty_fighting_wounding_shots);

        // p->addPriorityList(baseRotation, {});

        s->addBuff(std::make_unique<RelicProcBuff>(relic_mastery_surge, Mastery{2892}, Power{0}, CriticalRating{0.0}),
                   Second(0.0));
        if (!critRelic) {
            s->addBuff(
                std::make_unique<RelicProcBuff>(relic_power_surge, Mastery{0.0}, Power{2892}, CriticalRating{0.0}),
                Second(0.0));
        } else {
            s->addBuff(
                std::make_unique<RelicProcBuff>(relic_critical_surge, Mastery{0.0}, Power{0.0}, CriticalRating{2892}),
                Second(0.0));
        }
        addBuffs(s, c->getStaticBuffs(), Second(0.0));
        s->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
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
        rot.setMinTimeAfterInstant(Second(0.03));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.doRotation();

        return t->getEvents();
    };
    double totalStats = 2331.0 + 2095;
    struct info {
        AlacrityRating alacrity;
        CriticalRating crit;
        Mastery mastery;
        double mean;
        double stddev;
    };
    int iterations = 1;
    int stepSize = 40;
    int count = static_cast<int>(totalStats / 40);
    int masteryLoops = 14;
    int totalCount = count * masteryLoops;
    std::vector<info> infosCrit(totalCount);
    tbb::parallel_for(tbb::blocked_range<int>(0, count), [&](const tbb::blocked_range<int> &r) {
        for (int jj = 0; jj < masteryLoops; ++jj) {
            double mastery = 108 * jj;
            for (int ii = r.begin(); ii < r.end(); ++ii) {
                double alacrity = ii * stepSize;
                std::cout << "Alacrity: " << alacrity << " Mastery: " << mastery << "\n";
                double crit = std::max(0.0, totalStats - alacrity - mastery);
                std::vector<Second> times;
                for (int iit = 0; iit < iterations; ++iit) {
                    auto &&events = lambda(true, true, true, true, alacrity, crit, mastery, true);
                    auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
                    times.push_back(time);
                }
                auto &&[mean, stddev, minV, maxV] = getStdDev(times);
                infosCrit[ii + jj * count] =
                    info{AlacrityRating(alacrity), CriticalRating(crit), Mastery(mastery), mean.getValue(), stddev};
            }
        }
    });

    std::vector<info> infosPower(totalCount);
    tbb::parallel_for(tbb::blocked_range<int>(0, count), [&](const tbb::blocked_range<int> &r) {
        for (int jj = 0; jj < masteryLoops; ++jj) {
            double mastery = 108 * jj;
            for (int ii = r.begin(); ii < r.end(); ++ii) {
                double alacrity = ii * stepSize;
                std::cout << "Alacrity: " << alacrity << " Mastery: " << mastery << "\n";
                double crit = std::max(0.0, totalStats - alacrity - mastery);
                std::vector<Second> times;
                for (int iit = 0; iit < iterations; ++iit) {
                    auto &&events = lambda(true, true, true, true, alacrity, crit, mastery, true);
                    auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
                    times.push_back(time);
                }
                auto &&[mean, stddev, minV, maxV] = getStdDev(times);
                infosPower[ii + jj * count] =
                    info{AlacrityRating(alacrity), CriticalRating(crit), Mastery(mastery), mean.getValue(), stddev};
            }
        }
    });
    std::ofstream f("log.csv");
    f << "alacrity rating,critical rating,mastery bonus,mean (crit relic),stddev (crit relic),mean (power "
         "relic),stddev "
         "(power relic)"
      << "\n";
    for (int ii = 0; ii < infosCrit.size(); ++ii) {
        auto &&vc = infosCrit[ii];
        auto &&vp = infosPower[ii];
        f << vc.alacrity.getValue() << "," << vc.crit.getValue() << " , " << vc.mastery.getValue() << "," << vc.mean
          << "," << vc.stddev << "," << vp.mean << "," << vp.stddev << "\n";
    }
    f.close();
    detail::getLogger()->set_level(spdlog::level::info);
    double alacrity = 3210;
    double crit = std::max(0.0, totalStats - alacrity);
    auto &&events = lambda(true, true, true, true, alacrity, crit, false);
    auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
    logParseInformation(events, time);
    auto &&[s, t, c] = getTestData(alacrity, crit, true);
    auto abl = c->getAbility(smuggler_flurry_of_bolts);
    auto afs = getAllFinalStats(*abl, s, t);
    CHECK(afs[0].armorDebuff == false);
    t->addDebuff(detail::getGenericDebuff(debuff_shattered), s, Second(0.0));
    afs = getAllFinalStats(*abl, s, t);
    CHECK(afs[0].armorDebuff == true);
    delete d;

    /* can be ploted using the following python code
    import numpy as np
    import matplotlib.pyplot as plt
    a,c,m,mc,stdc,mp,stdp = np.loadtxt('log.csv',delimiter=',',unpack=True,skiprows=1)
    fig = plt.figure()
    ax1 = fig.add_subplot(111,projection='3d')
    ax1.set_xlabel("alacrity rating")
    ax1.set_ylabel("bonus matery")
    ax1.set_zlabel("TTK")
    ax1.scatter(a,m,mc)
    ax1.scatter(a,m,mp)
    fig.show()
    */
}

TEST(Serialize, WS2) {
    auto p = std::make_shared<PriorityList>();
    p->addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
    p->addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
    p->addAbility(gunslinger_illegal_mods, getCooldownFinishedCondition(gunslinger_illegal_mods));

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
    std::cout << std::setw(0) << p->serialize() << std::endl;
}

TEST(Serialize, WS3) {
    auto p = std::make_shared<PriorityList>();
    p->addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
    p->addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
    p->addAbility(gunslinger_illegal_mods, getCooldownFinishedCondition(gunslinger_illegal_mods));

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

    baseRotation->addAbility(gunslinger_quickdraw);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);

    baseRotation->addAbility(gunslinger_vital_shot);
    baseRotation->addAbility(dirty_fighting_shrap_bomb);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);

    baseRotation->addAbility(gunslinger_quickdraw);
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
    std::cout << std::setw(0) << p->serialize() << std::endl;
}
TEST(Serialize, WS3Snapshot) {
    auto p = std::make_shared<PriorityList>();
    p->addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
    p->addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
    p->addAbility(gunslinger_illegal_mods, getCooldownFinishedCondition(gunslinger_illegal_mods));

    auto baseRotation = std::make_shared<StaticRotation>();

    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(gunslinger_vital_shot);
    baseRotation->addAbility(dirty_fighting_shrap_bomb);
    baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);

    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(gunslinger_take_cover);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);

    baseRotation->addAbility(gunslinger_quickdraw);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);

    baseRotation->addAbility(gunslinger_vital_shot);
    baseRotation->addAbility(dirty_fighting_shrap_bomb);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(gunslinger_take_cover);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);

    baseRotation->addAbility(gunslinger_quickdraw);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);

    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(gunslinger_take_cover);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);

    p->addPriorityList(baseRotation, {});
    std::cout << std::setw(0) << p->serialize() << std::endl;
}