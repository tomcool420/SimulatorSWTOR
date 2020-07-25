#include "../DirtyFighting.h"
#include "../../libSimulator/Target.h"
#include "../detail/shared.h"
#include <gtest/gtest.h>
#include "../../libSimulator/detail/log.h"

using namespace Simulator;
namespace {
struct TestData{
    TargetPtr source;
    TargetPtr target;
    std::shared_ptr<DirtyFighting> df;
};
TestData getTestData(){
    RawStats rs;
    rs.master=Mastery{4953};
    rs.power=Power{2100};
    rs.accuracyRating=AccuracyRating{1557};
    rs.criticalRating=CriticalRating{313};
    rs.alacrityRating=AlacrityRating{0};
    rs.weaponDamageMH={1376.0,2556.0};
    rs.weaponDamageOH={1376.0*0.3,2556.0*0.3};
    rs.forceTechPower=FTPower{7008};
    TestData ret;
    ret.source=Target::New(rs);
    ret.target=Target::New(rs);
    ret.df=std::make_shared<DirtyFighting>();
    return ret;
}
}
TEST(DirtyFighting, DirtyBlast) {
    detail::LogDisabler d;
    auto && [player,target,df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false),Second(0.0));
    auto dirtyBlast = df->getAbility(dirty_fighting_dirty_blast);
    auto afs = getAllFinalStats(*dirtyBlast, player, target);
    auto DamageRange = calculateDamageRange(*dirtyBlast, afs);
    ASSERT_EQ(DamageRange.size(),3);
    ASSERT_NEAR(std::round(DamageRange[0].dmg.first+DamageRange[1].dmg.first),4829,1.0);
    ASSERT_NEAR(std::round(DamageRange[0].dmg.second+DamageRange[1].dmg.second),6095,1.0);
    ASSERT_NEAR(std::round(DamageRange[2].dmg.second),2428,1.0);
    ASSERT_NEAR(std::round(DamageRange[2].dmg.first),2428,1.0);
}
TEST(DirtyFighting, VitalShot) {
    detail::LogDisabler d;
    auto && [player,target,df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false),Second(0.0));
    auto abl = df->getAbility(gunslinger_vital_shot);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(),1);
    ASSERT_NEAR(std::round(DamageRange[0].dmg.first*7),9502,1.0); // Only 7 ticks on tooltip (there are actually 9 in parse)
    ASSERT_NEAR(std::round(DamageRange[0].dmg.second*7),9502,1.0);
}
TEST(DirtyFighting, SpeedShot) {
    detail::LogDisabler d;
    auto && [player,target,df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false),Second(0.0));
    auto abl = df->getAbility(gunslinger_speed_shot);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(),2);
    auto info = abl->getInfo();
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first+DamageRange[1].dmg.first)*info.nTicks),15289,1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second+DamageRange[1].dmg.second)*info.nTicks),19284,1.0);
}
TEST(DirtyFighting, WoundingShots) {
    detail::LogDisabler d;
    auto && [player,target,df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false),Second(0.0));
    auto abl = df->getAbility(dirty_fighting_wounding_shots);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(),2);
    auto info = abl->getInfo();
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first+DamageRange[1].dmg.first)),2020,1.0); // Damage is per tick on tooltip
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second+DamageRange[1].dmg.second)),2552,1.0);
}
TEST(DirtyFighting, ShrapBomb) {
    detail::LogDisabler d;
    auto && [player,target,df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false),Second(0.0));
    auto abl = df->getAbility(dirty_fighting_shrap_bomb);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(),1);
    auto info = abl->getInfo();
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first)),1251,1.0); // Damage for first tick
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second)),1251,1.0);
    
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first)*info.nTicks),10011,1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second)*info.nTicks),10011,1.0);
}
TEST(DirtyFighting, QuickDraw) {
    detail::LogDisabler d;
    auto && [player,target,df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false),Second(0.0));
    auto abl = df->getAbility(gunslinger_quickdraw);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(),2);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first+DamageRange[1].dmg.first)),9819,1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second+DamageRange[1].dmg.second)),12381,1.0);
}
TEST(DirtyFighting, HemoBlast) {
    detail::LogDisabler d;
    auto && [player,target,df] = getTestData();
    df->setExploitedWeakness(true);
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false),Second(0.0));
    auto abl = df->getAbility(dirty_fighting_hemorraghing_blast);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(),2);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first+DamageRange[1].dmg.first)),1022,1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second+DamageRange[1].dmg.second)),1284,1.0);
}
