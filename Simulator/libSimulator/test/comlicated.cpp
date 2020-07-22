#include "../detail/helpers.h"
#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../DOT.h"
#include "../StatBuff.h"
#include "../Target.h"
#include "../utility.h"
#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>
#include "../detail/log.h"
#include "../Rotation.h"
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
    addBuffs(player, std::move(buffs));

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

    player->addBuff(std::move(triumph));

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

    EXPECT_ANY_THROW(t->getDebuff<DOT>(tactics_gut_dot + 1, player->getId()));
    auto addedDot = t->getDebuff<DOT>(tactics_gut_dot, player->getId());
    EXPECT_TRUE(addedDot != nullptr);
    t->logHits();
    auto ne = t->getNextEventTime();
    CHECK(ne);
    Second lastAppliedTime = Second(0.0);
    ASSERT_TRUE(t->isBleeding());
    while (auto ine = t->getNextEventTime()) {
        Second nextRefreshTime = lastAppliedTime + Second(8.301);
        if (nextRefreshTime < *ine && t->getCurrentHealth() > HealthPoints(1e-6)) {
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
    addBuffs(player, std::move(buffs));
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

int countHits(AbilityId id,const std::vector<std::pair<Second, DamageHits>> &allHits){
    int cnt{0};
    for(auto && hits : allHits){
        for(const auto & hit : hits.second){
            if(hit.id==id){
                ++cnt;
            }
        }
    }
    return cnt;
}
TEST(Calculations,Rotation){
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
    s.hp = HealthPoints(2e6);
    auto target = Target::New(s);
    auto player = Target::New(rs);
    player->addBuff(getDefaultStatsBuffPtr(false, false));
    detail::LogDisabler d;
    {
        target->clearHits();
        std::vector<AbilityId> actions {dirty_fighting_dirty_blast};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto && hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 3);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 6);

    }
    {
        target->clearHits();
        std::vector<AbilityId> actions {dirty_fighting_dirty_blast,dirty_fighting_dirty_blast};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto && hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 6);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 7);
    }
    {
        target->clearHits();
        std::vector<AbilityId> actions {dirty_fighting_dirty_blast,dirty_fighting_wounding_shots};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto && hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 3);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 10);
        ASSERT_EQ(countHits(dirty_fighting_wounding_shots, hits), 8);
    }
    {
        target->clearHits();
        std::vector<AbilityId> actions {dirty_fighting_dirty_blast,dirty_fighting_hemorraghing_blast,dirty_fighting_wounding_shots};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto && hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 3);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 10);
        ASSERT_EQ(countHits(dirty_fighting_wounding_shots, hits), 8);
        ASSERT_EQ(countHits(dirty_fighting_hemorraghing_blast, hits), 16);
    }
    {
        target->clearHits();
        std::vector<AbilityId> actions {dirty_fighting_dirty_blast,dirty_fighting_hemorraghing_blast,dirty_fighting_wounding_shots,dirty_fighting_dirty_blast};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.doRotation();
        auto && hits = target->getHits();
        ASSERT_EQ(countHits(dirty_fighting_dirty_blast, hits), 6);
        ASSERT_EQ(countHits(dirty_fighting_exploited_weakness, hits), 13);
        ASSERT_EQ(countHits(dirty_fighting_wounding_shots, hits), 8);
        ASSERT_EQ(countHits(dirty_fighting_hemorraghing_blast, hits), 20);
    }
    {
        target->clearHits();
        std::vector<AbilityId> actions {tactics_tactical_surge,tactics_tactical_surge,tactics_tactical_surge,trooper_high_impact_bolt};
        SetRotation rot(player, std::move(actions));
        rot.setDelayAfterChanneled(Second(0.05));
        rot.setStart(Second(0.0));
        rot.setTarget(target);
        rot.setRepeats(3);
        rot.doRotation();
        auto && hits = target->getHits();
        ASSERT_EQ(countHits(tactics_tactical_surge, hits), 9);
        ASSERT_EQ(countHits(trooper_high_impact_bolt, hits), 3);
    }
}
TEST(Calculations, CausedDebuffs){
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
    player->addBuff(getDefaultStatsBuffPtr(false, false));
    
    std::vector<std::pair<Second,AbilityId>> actions{
        {Second(0.0),gunslinger_vital_shot}
                                                    ,{Second(1.5),dirty_fighting_shrap_bomb}
                                                    ,{Second(3.0),dirty_fighting_dirty_blast}
                                                    ,{Second(4.5),dirty_fighting_hemorraghing_blast}
                                                    ,{Second(6.0),dirty_fighting_dirty_blast}
    };
    int actionCounter = 0;
    auto ine = target->getNextEventTime();
    while(actionCounter<actions.size() || ine.has_value()){
        if(actionCounter<actions.size() && (!ine || *ine>actions[actionCounter].first)){
            auto ability = getAbility(actions[actionCounter].second);
            CHECK(ability);
            auto fs = getAllFinalStats(*ability, player, target);
            auto hits = getHits(*ability, fs, target);
            applyDamageToTarget(hits, player, target, actions[actionCounter].first);
            ability->onAbilityHitTarget(hits, player, target, actions[actionCounter].first);
            ++actionCounter;
        }else{
            target->applyEventsAtTime(*ine+Second(0.0001));
        }
        ine = target->getNextEventTime();
    }
}

TEST(Calculations, RawCellBurst){
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
    player->addBuff(getDefaultStatsBuffPtr(false, false));

    auto babl = getAbility(tactics_cell_burst);
    auto validateCellBurstRange = [&](int stacks, double min, double max){
        auto coefficients = babl->getCoefficients()[0];
        coefficients.multiplier = 0.1 + 1.05*(stacks-1);
        auto abl = std::make_shared<Ability>(tactics_cell_burst,AbilityInfo{{coefficients}});
        auto afs = getAllFinalStats(*abl, player, target);
        afs[0].forceTechCritChance=0.0;
        auto dmgRamge = calculateDamageRange(*abl, afs);
        EXPECT_NEAR(std::round(dmgRamge[0].dmg.first),min,1e-2);
        EXPECT_NEAR(std::round(dmgRamge[0].dmg.second),max,1e-2);
    };
    
    validateCellBurstRange(1,3252,3381);
    validateCellBurstRange(2,6356,6609);
    validateCellBurstRange(3,9460,9836);
    validateCellBurstRange(4,12564,13063);
    
    addBuffs(player, std::move(buffs));
    
    auto validateCellBurstRangeUsingBuff = [&](int stacks, double min, double max){
        auto coefficients = babl->getCoefficients()[0];
        coefficients.multiplier = 0.0;
        auto buff = player->getBuff<EnergyLodeBuff>(tactics_high_energy_cell);
        ASSERT_TRUE(buff);
        buff->setStacks(stacks);
        auto abl = std::make_shared<Ability>(tactics_cell_burst,AbilityInfo{{coefficients}});
        auto afs = getAllFinalStats(*abl, player, target);
        afs[0].forceTechCritChance=0.0;
        auto dmgRamge = calculateDamageRange(*abl, afs);
        EXPECT_NEAR(std::round(dmgRamge[0].dmg.first),min,1e-2);
        EXPECT_NEAR(std::round(dmgRamge[0].dmg.second),max,1e-2);
    };
    validateCellBurstRangeUsingBuff(0,3252,3381);
    validateCellBurstRangeUsingBuff(1,3252,3381);
    validateCellBurstRangeUsingBuff(2,6356,6609);
    validateCellBurstRangeUsingBuff(3,9460,9836);
    validateCellBurstRangeUsingBuff(4,12564,13063);
    //three stack 3252-3381
    //two stack 6356-6609
    //three stack 9460-9836
    //four stack 12564-13063
}
TEST(Calculations, CellBurst){
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
    player->addBuff(getDefaultStatsBuffPtr(false, false));
    addBuffs(player, std::move(buffs));
    std::vector<std::pair<Second,AbilityId>> actions{
        {Second(0.0),tactics_cell_burst}
    };
    int actionCounter = 0;
    auto ine = target->getNextEventTime();
    while(actionCounter<actions.size() || ine.has_value()){
        if(actionCounter<actions.size() && (!ine || *ine>actions[actionCounter].first)){
            auto ability = getAbility(actions[actionCounter].second);
            CHECK(ability);
            auto fs = getAllFinalStats(*ability, player, target);
            auto hits = getHits(*ability, fs, target);
            applyDamageToTarget(hits, player, target, actions[actionCounter].first);
            ability->onAbilityHitTarget(hits, player, target, actions[actionCounter].first);
            ++actionCounter;
        }else{
            target->applyEventsAtTime(*ine+Second(0.0001));
        }
        ine = target->getNextEventTime();
    }
    

    
}
