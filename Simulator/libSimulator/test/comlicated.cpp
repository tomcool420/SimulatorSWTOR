#include "helpers.h"
#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../DOT.h"
#include "../StatBuff.h"
#include "../Target.h"
#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>
#include "../utility.h"
using namespace Simulator;

TEST(calculations, complicated) {
    const auto sb = getDefaultStatsBuffs(false);
    auto rs = getDefaultStats();
    auto buffs = getTacticsSheetBuffs();
    RawStats s;
    Target t(s);
    auto player = std::make_shared<Target>(rs);
    auto gutBleeding = std::make_unique<DOT>(tactics_gut_dot, 0.25, 0.025, 0.025, 0, DamageType::Internal, true, false,
                                             7, Second(3), true);
    gutBleeding->setBleeding(true);
    gutBleeding->setSource(player);
    auto sc = sb;
    for (auto &&b : buffs) {
        b->apply(gutBleeding->getAbility(), sc, t);
    }
    
    auto gStats = getFinalStats(rs, sc);
    buffs.push_back(getDefaultStatsBuffPtr());
    addBuffs(*player, std::move(buffs));

    auto damageRange = calculateDamageRange(gutBleeding->getAbility(), gStats);
    std::vector<DamageHits> allhits;
    int hitCount = 10000;
    for(int ii = 0;ii<hitCount;++ii){
        allhits.push_back(adjustForHitsAndCrits(damageRange, gStats, t));
    }
    double minNorm = 1e20;
    double maxNorm = 0;
    double minCrit = 1e20;
    double maxCrit = 0;
    int critCount = 0;
    int missCount = 0;
    for(auto && hits : allhits){
        for(auto && hit : hits){
            hit.dmg*=1.05;// this is the bleeding debuff on target from triumph
            if(hit.miss){
                ++missCount;
            }
            else if(hit.crit){
                minCrit=std::min(minCrit,hit.dmg);
                maxCrit=std::max(maxCrit,hit.dmg);
                ++critCount;
            }else{
                minNorm=std::min(minNorm,hit.dmg);
                maxNorm=std::max(maxNorm,hit.dmg);
            }
        }
    }
    std::cout<<fmt::format("without damage bonus: {}, ratio to 1178: {}",minNorm/(1+gStats.multiplier),1178.0/(minNorm/(1+gStats.multiplier)))<<std::endl;
    std::cout<<fmt::format("After {} hits, there were {}% crits,  {}% misses",hitCount,100.0*critCount/hitCount,100.0*missCount/hitCount)<<std::endl;
    std::cout<<fmt::format("normal hit range: {} - {}, crit range: {} - {}",minNorm,maxNorm,minCrit,maxCrit)<<std::endl;
    bool checkTriumphLastHit{true};
    BuffPtr triumph = std::unique_ptr<Buff>(MakeOnAbilityHitBuff("Triumph",[&](DamageHits &hits, const Second &, Target &, Target &target)->DamageHits{
        bool isBleeding = target.isBleeding();
        checkTriumphLastHit= isBleeding; // this is just debugging code to test :)
        if(!isBleeding)
            return {};
        for(auto && hit : hits){
            std::cout<<fmt::format("Bleed is found on target, multiplying damage by 1.05")<<std::endl;
            hit.dmg*=1.05;
        }
        return {};
    }));
    
    player->addBuff(std::move(triumph));

    
    auto checkTriumph = [&](bool expected){
            auto hib = getAbility(trooper_high_impact_bolt);
            auto fsHiB = getFinalStats(*hib, *player, t);
            auto dmg = getHits(*hib, fsHiB, t);
            applyDamageToTarget(dmg, *player, t, Second(0.0));
            ASSERT_EQ(checkTriumphLastHit,expected);
        
    };
    checkTriumph(false);
    t.addDOT(std::move(gutBleeding),player, gStats, Second(0.0));
    checkTriumph(true);
    
    EXPECT_ANY_THROW(t.getDebuff<DOT>(tactics_gut_dot+1, player->getId()));
    auto addedDot = t.getDebuff<DOT>(tactics_gut_dot, player->getId());
    EXPECT_TRUE(addedDot!=nullptr);
    t.logHits();
    auto ne = t.getNextEventTime();
    CHECK(ne);
    Second lastAppliedTime = Second(0.0);
    ASSERT_TRUE(t.isBleeding());
    while(auto ine = t.getNextEventTime()){
        Second nextRefreshTime =lastAppliedTime+Second(8.301);
        if(nextRefreshTime<*ine && nextRefreshTime<Second(60)){
            lastAppliedTime = nextRefreshTime;
            t.refreshDOT(addedDot->getId(), player->getId(), lastAppliedTime);
        }else{
            t.applyEventsAtTime(*ine + Second(0.0001));
            
        }
        ASSERT_TRUE(checkTriumphLastHit);
    }
    ASSERT_FALSE(t.isBleeding());
    checkTriumph(false);
    t.logHits();

}
