#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../AbilityDebuff.h"
#include "../DOT.h"
#include "../Rotation.h"
#include "../StatBuff.h"
#include "../Target.h"
#include "../detail/helpers.h"
#include "../detail/log.h"
#include "../utility.h"
#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>

using namespace Simulator;

TEST(calculations, complicated) {
    const auto sb = getDefaultStatsBuffs(false);
    auto rs = getDefaultStats();
    auto buffs = getTacticsSheetBuffs();
    RawStats s;
    s.hp = HealthPoints(2e6);
    auto t = Target::New(s);
    auto player = Target::New(rs);
    auto gutBleeding = std::make_unique<DOT>(tactics_gut_dot, 0.25, 0.025, 0.025, 0, DamageType::Internal, true, false,
                                             7, Second(3), true);
    gutBleeding->setBleeding(true);
    gutBleeding->setSource(player);
    auto sc = sb;
    AllStatChanges asc{sc};
    for (auto &&b : buffs) {
        b->apply(gutBleeding->getAbility(), asc, t);
    }

    auto gStats = getFinalStats(rs, sc);
    buffs.push_back(getDefaultStatsBuffPtr());
    addBuffs(player, std::move(buffs), Second(0.0));

    auto damageRange = calculateDamageRange(gutBleeding->getAbility(), {gStats});
    std::vector<DamageHits> allhits;
    int hitCount = 10000;
    for (int ii = 0; ii < hitCount; ++ii) {
        allhits.push_back(adjustForHitsAndCrits(damageRange, {gStats}, t));
    }
    double minNorm = 1e20;
    double maxNorm = 0;
    double minCrit = 1e20;
    double maxCrit = 0;
    int critCount = 0;
    int missCount = 0;
    for (auto &&hits : allhits) {
        for (auto &&hit : hits) {
            hit.dmg *= 1.05; // this is the bleeding debuff on target from triumph
            if (hit.miss) {
                ++missCount;
            } else if (hit.crit) {
                minCrit = std::min(minCrit, hit.dmg);
                maxCrit = std::max(maxCrit, hit.dmg);
                ++critCount;
            } else {
                minNorm = std::min(minNorm, hit.dmg);
                maxNorm = std::max(maxNorm, hit.dmg);
            }
        }
    }
    std::cout << fmt::format("without damage bonus: {}, ratio to 1178: {}", minNorm / (1 + gStats.multiplier),
                             1178.0 / (minNorm / (1 + gStats.multiplier)))
              << std::endl;
    std::cout << fmt::format("After {} hits, there were {}% crits,  {}% misses", hitCount, 100.0 * critCount / hitCount,
                             100.0 * missCount / hitCount)
              << std::endl;
    std::cout << fmt::format("normal hit range: {} - {}, crit range: {} - {}", minNorm, maxNorm, minCrit, maxCrit)
              << std::endl;
    bool checkTriumphLastHit{true};
    BuffPtr triumph = std::unique_ptr<Buff>(MakeOnAbilityHitBuff(
        "Triumph", [&](DamageHits &hits, const Second &, const TargetPtr &, const TargetPtr &target) -> DamageHits {
            bool isBleeding = target->isBleeding();
            checkTriumphLastHit = isBleeding; // this is just debugging code to test :)
            if (!isBleeding)
                return {};
            for (auto &&hit : hits) {
                hit.dmg *= 1.05;
            }
            return {};
        }));

    player->addBuff(std::move(triumph), Second(0.0));

    auto checkTriumph = [&](bool expected) {
        auto hib = getAbility(trooper_high_impact_bolt);
        auto fsHiB = getAllFinalStats(*hib, player, t);
        auto dmg = getHits(*hib, fsHiB, t);
        applyDamageToTarget(dmg, player, t, Second(0.0));
        ASSERT_EQ(checkTriumphLastHit, expected);
    };
    checkTriumph(false);
    t->addDOT(std::move(gutBleeding), player, {gStats}, Second(0.0));
    checkTriumph(true);

    EXPECT_EQ(t->getDebuff<DOT>(tactics_gut_dot + 1, player->getId()), nullptr);
    auto addedDot = t->getDebuff<DOT>(tactics_gut_dot, player->getId());
    EXPECT_TRUE(addedDot != nullptr);
    t->logHits();
    auto ne = t->getNextEventTime();
    CHECK(ne);
    Second lastAppliedTime = Second(0.0);
    ASSERT_TRUE(t->isBleeding());
    while (auto ine = t->getNextEventTime()) {
        Second nextRefreshTime = lastAppliedTime + Second(8.301);
        if (nextRefreshTime < *ine && t->getCurrentHealth() > HealthPoints(100000)) {
            lastAppliedTime = nextRefreshTime;
            t->refreshDOT(addedDot->getId(), player->getId(), lastAppliedTime);
        } else {
            t->applyEventsAtTime(*ine + Second(0.0001));
        }
        ASSERT_TRUE(checkTriumphLastHit);
    }
    ASSERT_FALSE(t->isBleeding());
    checkTriumph(false);
    t->logHits();
}

TEST(Calculations, HemoBlast) {
    auto rs = getDefaultStats();
    auto buffs = getTacticsSheetBuffs();
    rs.hasOffhand = true;
    rs.weaponDamageOH = {1573, 2359};
    RawStats s;
    s.hp = HealthPoints(2e6);
    auto target = Target::New(s);
    auto player = Target::New(rs);
    auto gutBleeding = std::make_unique<DOT>(tactics_gut_dot, 0.25, 0.025, 0.025, 0, DamageType::Internal, true, false,
                                             7, Second(3), true);
    gutBleeding->setBleeding(true);
    gutBleeding->setSource(player);
    buffs.push_back(getDefaultStatsBuffPtr());
    addBuffs(player, std::move(buffs), Second(0.0));
    auto dotStats = getAllFinalStats(gutBleeding->getAbility(), player, target);
    target->addDOT(std::move(gutBleeding), player, dotStats, Second(0.0));

    {
        auto hemoBlast = getAbility(dirty_fighting_hemorraghing_blast);
        auto fs = getAllFinalStats(*hemoBlast, player, target);
        auto hits = getHits(*hemoBlast, fs, target);
        target->applyDamageHit(hits, target, Second{1.5});
        hemoBlast->onAbilityHitTarget(hits, player, target, Second{1.5});
    }
    CHECK(target->getDebuff<Debuff>(dirty_fighting_hemorraghing_blast, player->getId()));
    Second lastAppliedTime = Second(0.0);
    auto addedDot = target->getDebuff<DOT>(tactics_gut_dot, player->getId());
    CHECK(addedDot);
    while (auto ine = target->getNextEventTime()) {
        Second nextRefreshTime = lastAppliedTime + Second(8.301);
        if (nextRefreshTime < *ine && nextRefreshTime < Second(30)) {
            lastAppliedTime = nextRefreshTime;
            target->refreshDOT(addedDot->getId(), player->getId(), lastAppliedTime);
        } else {
            target->applyEventsAtTime(*ine + Second(0.0001));
        }
    }
    target->logHits();
}

int countHits(AbilityId id, const std::vector<std::pair<Second, DamageHits>> &allHits) {
    int cnt{0};
    for (auto &&hits : allHits) {
        for (const auto &hit : hits.second) {
            if (hit.id == id) {
                ++cnt;
            }
        }
    }
    return cnt;
}

TEST(Calculations, RelicProc) {
    RawStats rs;
    rs.master = Mastery(3063);
    rs.power = Power(1304);
    rs.criticalRating = CriticalRating(1012);
    rs.alacrityRating = AlacrityRating(2331);
    rs.accuracyRating = AccuracyRating(264);
    rs.forceTechPower = FTPower(7008);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {1376.0, 2556.0};
    rs.hasOffhand = false;
    RawStats s;
    s.hp = HealthPoints(1e6);
    auto target = Target::New(s);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false), Second(0.0));
    player->addBuff(std::make_unique<RelicProcBuff>(relic_mastery_surge, Mastery{2892}, Power{0}, CriticalRating{0.0}),
                    Second(0.0));
    player->addBuff(std::make_unique<RelicProcBuff>(relic_power_surge, Mastery{0.0}, Power{2892}, CriticalRating{0.0}),
                    Second(0.0));

    {
        target->clearHits();
        std::vector<AbilityId> actions{dirty_fighting_dirty_blast};
        SetRotation rot(player, std::move(actions));
        rot.setRepeats(20);
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
    }
    std::cout << "Target Health at the end: " << target->getCurrentHealth().getValue() << std::endl;
}

TEST(Calculations, Rotation2WS) {
    RawStats rs;
    rs.master = Mastery(4953);
    rs.power = Power(2100);
    rs.criticalRating = CriticalRating(529);
    rs.alacrityRating = AlacrityRating(0);
    rs.accuracyRating = AccuracyRating(1557);
    rs.forceTechPower = FTPower(7008);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {413, 767};
    rs.hasOffhand = false;
    RawStats s;
    s.hp = HealthPoints(1e6);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false), Second(0.0));
    //    player->addBuff(std::make_unique<RelicProcBuff>(relic_mastery_surge,Mastery{2892},Power{0},CriticalRating{0.0}),Second(0.0));
    //    player->addBuff(std::make_unique<RelicProcBuff>(relic_power_surge,Mastery{0.0},Power{2892},CriticalRating{0.0}),Second(0.0));
    std::vector<int> damages;
    for (int ii = 0; ii < 1000; ++ii) {
        detail::LogDisabler d;
        auto target = Target::New(s);
        target->clearHits();
        std::vector<AbilityId> actions{dirty_fighting_dirty_blast,    gunslinger_vital_shot,
                                       dirty_fighting_shrap_bomb,     dirty_fighting_hemorraghing_blast,
                                       dirty_fighting_wounding_shots, dirty_fighting_dirty_blast,
                                       dirty_fighting_dirty_blast,    dirty_fighting_dirty_blast,
                                       dirty_fighting_dirty_blast,    dirty_fighting_wounding_shots};
        SetRotation rot(player, std::move(actions));
        rot.setRepeats(1);
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        damages.push_back(static_cast<int>(target->getMaxHealth().getValue() - target->getCurrentHealth().getValue()));
    }
    std::sort(damages.begin(), damages.end());
    auto sum = std::accumulate(damages.begin(), damages.end(), 0);
    SIM_INFO("Over {} iterations, mean damage is {} and range is {} - {}", damages.size(), double(sum) / damages.size(),
             damages.front(), damages.back());
}

std::set<double> getDamageHits(const Target::TargetEvents &events, AbilityId id) {
    std::set<double> ret;
    for (auto &&event : events) {
        if (event.type == Target::TargetEventType::Damage) {
            for (auto &&hit : *event.damage) {
                if (hit.id == id) {
                    ret.insert(hit.dmg);
                }
            }
        }
    }
    std::cout << fmt::format("Hits for ability [{} {}] are : ", detail::getAbilityName(id), id);
    for (auto &&i : ret) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    return ret;
}

std::map<AbilityId, AbilityLogInformation> getEventInformation(const TargetPtr &target) {
    std::map<AbilityId, AbilityLogInformation> ret;
    for (auto &&event : target->getEvents()) {
        if (event.type != Target::TargetEventType::Damage)
            continue;
        for (auto &&hit : *event.damage) {
            auto &&info = ret[hit.id];
            info.id = hit.id;
            info.totalDamage += hit.dmg;
            if (hit.crit)
                info.critCount += 1;
            else if (hit.miss)
                info.missCount += 1;
            else
                info.hitCount += 1;
        }
    }
    return ret;
}
void logParseInformation(const TargetPtr &target, Second duration) {
    auto abilities = getEventInformation(target);
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
}
TEST(Calculations, Rotation2WS_HighStats) {
    RawStats rs;
    rs.master = Mastery(12138);
    rs.power = Power(9393);
    rs.criticalRating = CriticalRating(1879);
    rs.alacrityRating = AlacrityRating(2331);
    rs.accuracyRating = AccuracyRating(1557);
    rs.forceTechPower = FTPower(7008);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {413, 767};
    rs.hasOffhand = false;
    RawStats s;
    s.hp = HealthPoints(3.25e6);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto ab = std::make_unique<AmplifierBuff>();
    ab->setPeriodicIntensityBonus(0.022 * 9 + 0.0088);
    //    ab->setArmorPenBonus(0.025*9);
    //    ab->setForcecTechWizardryBonus(0.09);
    player->addBuff(std::move(ab), Second(0.0));
    auto cb = std::make_unique<ColdBloodedBuff>();
    player->addBuff(std::move(cb), Second(0.0));
    //    player->addBuff(std::make_unique<RelicProcBuff>(relic_mastery_surge,Mastery{2892},Power{0},CriticalRating{0.0}),Second(0.0));
    //    player->addBuff(std::make_unique<RelicProcBuff>(relic_power_surge,Mastery{0.0},Power{2892},CriticalRating{0.0}),Second(0.0));
    std::vector<Second> deathTimes;
    for (int ii = 0; ii < 10; ++ii) {
        auto d = new detail::LogDisabler;
        auto target = Target::New(s);
        target->clearHits();
        //        std::vector<AbilityId>
        //        actions{dirty_fighting_wounding_shots,dirty_fighting_dirty_blast,gunslinger_quickdraw,gunslinger_quickdraw};
        std::vector<AbilityId> actions{dirty_fighting_dirty_blast,    gunslinger_vital_shot,
                                       dirty_fighting_shrap_bomb,     dirty_fighting_hemorraghing_blast,
                                       dirty_fighting_wounding_shots, dirty_fighting_dirty_blast,
                                       dirty_fighting_dirty_blast,    dirty_fighting_dirty_blast,
                                       gunslinger_quickdraw,          dirty_fighting_wounding_shots};
        SetRotation rot(player, std::move(actions));
        rot.setRepeats(50);
        rot.setDelayAfterChanneled(Second(0.2));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        deathTimes.push_back(target->getDeathTime().value());
        //    getDamageHits(target->getEvents(), gunslinger_vital_shot);
        //    getDamageHits(target->getEvents(), dirty_fighting_shrap_bomb);
        //    getDamageHits(target->getEvents(), dirty_fighting_bloody_mayhem);
        delete d;
        //    logParseInformation(target, *(target->getDeathTime())-Second(1.5));
    }
    std::sort(deathTimes.begin(), deathTimes.end());
    auto sum = std::accumulate(deathTimes.begin(), deathTimes.end(), Second(0));
    Second mean = sum / (double)deathTimes.size();
    SIM_INFO("Over {} iterations, mean death time is {} and range is {} - {}", deathTimes.size(),
             sum.getValue() / deathTimes.size(), deathTimes.front(), deathTimes.back());

    SIM_INFO("Over {} iterations, mean dps is {} and range is {}-{}", deathTimes.size(),
             s.hp.getValue() / (mean - Second(1.5)).getValue(),
             s.hp.getValue() / (deathTimes.back() - Second(1.5)).getValue(),
             s.hp.getValue() / (deathTimes.front() - Second(1.5)).getValue());
}

TEST(Calculations, DotsEntrenchedOffence) {
    RawStats rs;
    rs.master = Mastery(12138);
    rs.power = Power(9393);
    rs.criticalRating = CriticalRating(1879);
    rs.alacrityRating = AlacrityRating(2331);
    rs.accuracyRating = AccuracyRating(1557);
    rs.forceTechPower = FTPower(7008);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {413, 767};
    rs.hasOffhand = false;
    RawStats s;
    s.hp = HealthPoints(3.25e6);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto ab = std::make_unique<AmplifierBuff>();
    ab->setPeriodicIntensityBonus(0.022 * 9 + 0.0088);
    player->addBuff(std::move(ab), Second(0.0));
    auto cb = std::make_unique<ColdBloodedBuff>();
    player->addBuff(std::move(cb), Second(0.0));

    auto target = Target::New(s);
    target->addDebuff(std::make_unique<ShatteredDebuff>(), player, Second(0.0));
    std::vector<AbilityId> actions{gunslinger_hunker_down, gunslinger_vital_shot, dirty_fighting_shrap_bomb};
    SetRotation rot(player, std::move(actions));
    rot.setRepeats(1);
    rot.setDelayAfterChanneled(Second(0.2));
    rot.setStart(Second(0.0));
    rot.setTarget(target);
    rot.doRotation();
}

TEST(Calculations, Rotation) {
    RawStats rs;
    rs.master = Mastery(4953);
    rs.power = Power(2100);
    rs.criticalRating = CriticalRating(313);
    rs.alacrityRating = AlacrityRating(0);
    rs.accuracyRating = AccuracyRating(1557);
    rs.forceTechPower = FTPower(7008);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {1376.0, 2556.0};
    rs.hasOffhand = false;
    RawStats s;
    s.hp = HealthPoints(2e6);
    auto target = Target::New(s);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto d = new detail::LogDisabler;
    {
        target->clearHits();
        std::vector<AbilityId> actions{dirty_fighting_dirty_blast};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto &&hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 3);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 6);
    }
    {
        target->clearHits();
        std::vector<AbilityId> actions{dirty_fighting_dirty_blast, dirty_fighting_dirty_blast};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto &&hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 6);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 7);
    }

    {
        target->clearHits();
        std::vector<AbilityId> actions{dirty_fighting_dirty_blast, dirty_fighting_wounding_shots};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto &&hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 3);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 10);
        ASSERT_EQ(countHits(dirty_fighting_wounding_shots, hits), 8);
    }
    {
        target->clearHits();
        std::vector<AbilityId> actions{dirty_fighting_dirty_blast, dirty_fighting_hemorraghing_blast,
                                       dirty_fighting_wounding_shots};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto &&hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 3);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 10);
        ASSERT_EQ(countHits(dirty_fighting_wounding_shots, hits), 8);
        ASSERT_EQ(countHits(dirty_fighting_hemorraghing_blast, hits), 16);
    }
    {
        target->clearHits();
        std::vector<AbilityId> actions{dirty_fighting_dirty_blast, dirty_fighting_hemorraghing_blast,
                                       dirty_fighting_wounding_shots, dirty_fighting_dirty_blast};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto &&hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 6);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 13);
        ASSERT_EQ(countHits(dirty_fighting_wounding_shots, hits), 8);
        ASSERT_EQ(countHits(dirty_fighting_hemorraghing_blast, hits), 20);
    }
    {
        target->clearHits();
        std::vector<AbilityId> actions{tactics_tactical_surge, tactics_tactical_surge, tactics_tactical_surge,
                                       trooper_high_impact_bolt};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.setRepeats(3);
        rot.doRotation();
        auto &&hits = target->getHits();
        ASSERT_EQ(countHits(tactics_tactical_surge, hits), 9);
        ASSERT_EQ(countHits(trooper_high_impact_bolt, hits), 3);
    }

    {
        target->clearHits();
        std::vector<AbilityId> actions{gunslinger_smugglers_luck, tactics_tactical_surge, dirty_fighting_wounding_shots,
                                       tactics_tactical_surge,    tactics_tactical_surge, trooper_high_impact_bolt};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.setRepeats(1);
        rot.doRotation();
        auto &&allHits = target->getHits();
        for (auto &&hits : allHits) {
            for (auto &&hit : hits.second) {
                if (hit.id == dirty_fighting_wounding_shots && hit.miss == false) {
                    EXPECT_TRUE(hit.crit);
                }
            }
        }
    }
    delete d;

    {
        target->clearHits();
        target->addDebuff(std::make_unique<ShatteredDebuff>(), player, Second(0.0));
        std::vector<AbilityId> actions{gunslinger_smugglers_luck, gunslinger_hunker_down, dirty_fighting_shrap_bomb,
                                       dirty_fighting_wounding_shots, dirty_fighting_shrap_bomb};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.setRepeats(1);
        rot.doRotation();
        auto &&allHits = target->getHits();
        for (auto &&hits : allHits) {
            for (auto &&hit : hits.second) {
                if (hit.id == dirty_fighting_wounding_shots && hit.miss == false) {
                    EXPECT_TRUE(hit.crit);
                }
            }
        }
    }
}
TEST(Calculations, CausedDebuffs) {
    RawStats rs;
    rs.master = Mastery(3063);
    rs.power = Power(1304);
    rs.criticalRating = CriticalRating(1012);
    rs.alacrityRating = AlacrityRating(2331);
    rs.accuracyRating = AccuracyRating(264);
    rs.forceTechPower = FTPower(7008);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {1376.0, 2556.0};
    rs.hasOffhand = true;
    RawStats s;
    s.hp = HealthPoints(2e6);
    auto target = Target::New(s);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false), Second(0.0));

    std::vector<std::pair<Second, AbilityId>> actions{{Second(0.0), gunslinger_vital_shot},
                                                      {Second(1.5), dirty_fighting_shrap_bomb},
                                                      {Second(3.0), dirty_fighting_dirty_blast},
                                                      {Second(4.5), dirty_fighting_hemorraghing_blast},
                                                      {Second(6.0), dirty_fighting_dirty_blast}};
    int actionCounter = 0;
    auto ine = target->getNextEventTime();
    while (actionCounter < actions.size() || ine.has_value()) {
        if (actionCounter < actions.size() && (!ine || *ine > actions[actionCounter].first)) {
            auto ability = getAbility(actions[actionCounter].second);
            CHECK(ability);
            auto fs = getAllFinalStats(*ability, player, target);
            auto hits = getHits(*ability, fs, target);
            applyDamageToTarget(hits, player, target, actions[actionCounter].first);
            ability->onAbilityHitTarget(hits, player, target, actions[actionCounter].first);
            ++actionCounter;
        } else {
            target->applyEventsAtTime(*ine + Second(0.0001));
        }
        ine = target->getNextEventTime();
    }
}

TEST(Calculations, RawCellBurst) {
    RawStats rs;
    rs.master = Mastery(4814);
    rs.power = Power(2309);
    rs.criticalRating = CriticalRating(421);
    rs.alacrityRating = AlacrityRating(0);
    rs.accuracyRating = AccuracyRating(1557);
    rs.forceTechPower = FTPower(3504);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.hasOffhand = false;
    auto buffs = getTacticsSheetBuffs();
    RawStats s;
    s.hp = HealthPoints(2e6);
    auto target = Target::New(s);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false), Second(0.0));

    auto babl = getAbility(tactics_cell_burst);
    auto validateCellBurstRange = [&](int stacks, double min, double max) {
        auto coefficients = babl->getCoefficients()[0];
        coefficients.multiplier = 0.1 + 1.05 * (stacks - 1);
        auto abl = std::make_shared<Ability>(tactics_cell_burst, AbilityInfo{{coefficients}});
        auto afs = getAllFinalStats(*abl, player, target);
        afs[0].forceTechCritChance = 0.0;
        auto dmgRamge = calculateDamageRange(*abl, afs);
        EXPECT_NEAR(std::round(dmgRamge[0].dmg.first), min, 1e-2);
        EXPECT_NEAR(std::round(dmgRamge[0].dmg.second), max, 1e-2);
    };

    validateCellBurstRange(1, 3252, 3381);
    validateCellBurstRange(2, 6356, 6609);
    validateCellBurstRange(3, 9460, 9836);
    validateCellBurstRange(4, 12564, 13063);

    addBuffs(player, std::move(buffs), Second{0.0});

    auto validateCellBurstRangeUsingBuff = [&](int stacks, double min, double max) {
        auto coefficients = babl->getCoefficients()[0];
        coefficients.multiplier = 0.0;
        auto buff = player->getBuff<EnergyLodeBuff>(tactics_high_energy_cell);
        ASSERT_TRUE(buff);
        buff->setStacks(stacks);
        auto abl = std::make_shared<Ability>(tactics_cell_burst, AbilityInfo{{coefficients}});
        auto afs = getAllFinalStats(*abl, player, target);
        afs[0].forceTechCritChance = 0.0;
        auto dmgRamge = calculateDamageRange(*abl, afs);
        EXPECT_NEAR(std::round(dmgRamge[0].dmg.first), min, 1e-2);
        EXPECT_NEAR(std::round(dmgRamge[0].dmg.second), max, 1e-2);
    };
    validateCellBurstRangeUsingBuff(0, 3252, 3381);
    validateCellBurstRangeUsingBuff(1, 3252, 3381);
    validateCellBurstRangeUsingBuff(2, 6356, 6609);
    validateCellBurstRangeUsingBuff(3, 9460, 9836);
    validateCellBurstRangeUsingBuff(4, 12564, 13063);
    // three stack 3252-3381
    // two stack 6356-6609
    // three stack 9460-9836
    // four stack 12564-13063
}
TEST(Calculations, CellBurst) {
    RawStats rs;
    rs.master = Mastery(4814);
    rs.power = Power(2309);
    rs.criticalRating = CriticalRating(421);
    rs.alacrityRating = AlacrityRating(0);
    rs.accuracyRating = AccuracyRating(1557);
    rs.forceTechPower = FTPower(3504);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.hasOffhand = false;
    auto buffs = getTacticsSheetBuffs();
    RawStats s;
    s.hp = HealthPoints(2e6);
    auto target = Target::New(s);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false), Second(0.0));
    addBuffs(player, std::move(buffs), Second{0.0});
    std::vector<std::pair<Second, AbilityId>> actions{{Second(0.0), tactics_cell_burst}};
    int actionCounter = 0;
    auto ine = target->getNextEventTime();
    while (actionCounter < actions.size() || ine.has_value()) {
        if (actionCounter < actions.size() && (!ine || *ine > actions[actionCounter].first)) {
            auto ability = getAbility(actions[actionCounter].second);
            CHECK(ability);
            auto fs = getAllFinalStats(*ability, player, target);
            auto hits = getHits(*ability, fs, target);
            applyDamageToTarget(hits, player, target, actions[actionCounter].first);
            ability->onAbilityHitTarget(hits, player, target, actions[actionCounter].first);
            ++actionCounter;
        } else {
            target->applyEventsAtTime(*ine + Second(0.0001));
        }
        ine = target->getNextEventTime();
    }
}
