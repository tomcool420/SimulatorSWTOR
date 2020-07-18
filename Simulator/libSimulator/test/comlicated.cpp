#include "helpers.h"
#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../DOT.h"
#include "../StatBuff.h"
#include "../Target.h"
#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>
#include "../utility.h"
#include "../Player.h"
using namespace Simulator;

TEST(calculations, complicated) {
    const auto sb = getDefaultStatsBuffs(false);
    auto rs = getDefaultStats();
    auto buffs = getTacticsSheetBuffs();
    Target t(1e6); // create a dummy with 1 mil HP;
    auto player = std::make_shared<Player>(rs);
    auto gutBleeding = std::make_unique<DOT>(2029878853501184, 0.25, 0.025, 0.025, 0, DamageType::Internal, true, false,
                                             7, Second(3), true);
    gutBleeding->setPlayer(player);
    auto sc = sb;
    for (auto &&b : buffs) {
        b->apply(*gutBleeding, sc, t);
    }
    
    auto gStats = getFinalStats(rs, sc);
    buffs.push_back(getDefaultStatsBuffPtr());
    player->setBuffs(std::move(buffs));

    auto damageRange = calculateDamageRange(*gutBleeding, gStats);
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
            hit.dmg*=1.05;
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
    
    auto && addedDot = t.addDOT(std::move(gutBleeding),player, gStats, Second(0.0));
    
    t.logHits();
    auto ne = t.getNextEventTime();
    CHECK(ne);
    Second lastAppliedTime = Second(0.0);
    while(auto ine = t.getNextEventTime()){
        if(*ine>Second(60))
            break;
        
        if(lastAppliedTime+Second(8.301)<*ine){
            lastAppliedTime = lastAppliedTime+Second(8.301);
            t.refreshDOT(addedDot->id, player->getId(), lastAppliedTime);            
        }else{
            t.applyEventsAtTime(*ine + Second(0.0001));
            
        }
        
        
    }
    t.logHits();

}
