#include "Simulator/SimulatorBase/Rotation.h"
#include "Simulator/SimulatorBase/Target.h"
#include "Simulator/SimulatorBase/detail/log.h"
#include "../DirtyFighting.h"
#include "../detail/shared.h"
#include <Simulator/SimulatorBase/parseHelpers.h>
#include <gtest/gtest.h>

using namespace Simulator;
namespace {
struct TestData {
    TargetPtr source;
    TargetPtr target;
    std::shared_ptr<DirtyFighting> df;
};
TestData getTestData() {
    RawStats rs;
    rs.master = Mastery{11795};
    rs.power = Power{8108};
    rs.accuracyRating = AccuracyRating{3329};
    rs.criticalRating = CriticalRating{1590};
    rs.alacrityRating = AlacrityRating{1019};
    rs.weaponDamageMH = {1596, 2964};
    rs.weaponDamageOH = {1376.0 * 0.25, 2556.0 * 0.25};
    rs.forceTechPower = FTPower{7568};
    TestData ret;
    ret.source = Target::New(rs);
    ret.target = Target::New(rs);
    ret.df = std::make_shared<DirtyFighting>();
    return ret;
}
TestData getTestDataHA() {
    RawStats rs;
    rs.master = Mastery{12138};
    rs.power = Power{10049};
    rs.accuracyRating = AccuracyRating{1590};
    rs.criticalRating = CriticalRating{2800};
    rs.alacrityRating = AlacrityRating{1500};
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {1376.0 * 0.3, 2556.0 * 0.3};
    rs.forceTechPower = FTPower{7008};
    TestData ret;
    ret.source = Target::New(rs);
    ret.target = Target::New(rs);
    ret.df = std::make_shared<DirtyFighting>();
    return ret;
}
} // namespace
TEST(DirtyFighting, DirtyBlast) {
    detail::LogDisabler d;
    auto &&[player, target, df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto dirtyBlast = df->getAbility(dirty_fighting_dirty_blast);
    auto afs = getAllFinalStats(*dirtyBlast, player, target);
    auto DamageRange = calculateDamageRange(*dirtyBlast, afs);
    ASSERT_EQ(DamageRange.size(), 3);
    EXPECT_NEAR(std::round(DamageRange[0].dmg.first + DamageRange[1].dmg.first), 9446, 1.0);
    EXPECT_NEAR(std::round(DamageRange[0].dmg.second + DamageRange[1].dmg.second), 10818, 1.0);
    EXPECT_NEAR(std::round(DamageRange[2].dmg.second), 4508, 1.0);
    EXPECT_NEAR(std::round(DamageRange[2].dmg.first), 4508, 1.0);
}
TEST(DirtyFighting, VitalShot) {
    detail::LogDisabler d;
    auto &&[player, target, df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto abl = df->getAbility(gunslinger_vital_shot);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(), 1);
    ASSERT_NEAR(std::round(DamageRange[0].dmg.first * 7), 17642,
                1.0); // Only 7 ticks on tooltip (there are actually 9 in parse)
    ASSERT_NEAR(std::round(DamageRange[0].dmg.second * 7), 17642, 1.0);
}
TEST(DirtyFighting, SpeedShot) {
    detail::LogDisabler d;
    auto &&[player, target, df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto abl = df->getAbility(gunslinger_speed_shot);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(), 2);
    auto info = abl->getInfo();
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first) * info.nTicks), 29927, 1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second) * info.nTicks), 34257, 1.0);
}
TEST(DirtyFighting, WoundingShots) {
    detail::LogDisabler d;
    auto &&[player, target, df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto abl = df->getAbility(dirty_fighting_wounding_shots);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(), 2);
    auto info = abl->getInfo();
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first)), 3949,
                1.0); // Damage is per tick on tooltip
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second)), 4525, 1.0);
}
TEST(DirtyFighting, ShrapBomb) {
    detail::LogDisabler d;
    auto &&[player, target, df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto abl = df->getAbility(dirty_fighting_shrap_bomb);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(), 1);
    auto info = abl->getInfo();
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first)), 2324, 1.0); // Damage for first tick
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second)), 2324, 1.0);

    ASSERT_NEAR(std::round((DamageRange[0].dmg.first) * info.nTicks), 18589, 1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second) * info.nTicks), 18589, 1.0);
}
TEST(DirtyFighting, QuickDraw) {
    detail::LogDisabler d;
    auto &&[player, target, df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto abl = df->getAbility(gunslinger_quickdraw);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(), 2);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first)), 19225, 1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second)), 22002, 1.0);
}
TEST(DirtyFighting, HemoBlast) {
    detail::LogDisabler d;
    {
        auto &&[player, target, df] = getTestData();
        df->setExploitedWeakness(false);
        addBuffs(player, df->getStaticBuffs(), Second(0.0));
        player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
        auto abl = df->getAbility(dirty_fighting_hemorraghing_blast);
        auto afs = getAllFinalStats(*abl, player, target);
        auto DamageRange = calculateDamageRange(*abl, afs);
        ASSERT_EQ(DamageRange.size(), 2);
        ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first)), 1361, 1.0);
        ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second)), 1553, 1.0);
    }
    {
        auto &&[player, target, df] = getTestData();
        df->setExploitedWeakness(true);
        addBuffs(player, df->getStaticBuffs(), Second(0.0));
        player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
        auto abl = df->getAbility(dirty_fighting_hemorraghing_blast);
        auto afs = getAllFinalStats(*abl, player, target);
        auto DamageRange = calculateDamageRange(*abl, afs);
        ASSERT_EQ(DamageRange.size(), 2);
        ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first)), 2009, 1.0);
        ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second)), 2292, 1.0);
    }
}

TEST(DirtyFighting, FlurryOfBolts) {
    detail::LogDisabler d;
    auto &&[player, target, df] = getTestData();
    df->setExploitedWeakness(true);
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto abl = df->getAbility(smuggler_flurry_of_bolts);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(), 2);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first)), 6375, 1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second)), 8038, 1.0);
}

TEST(DirtyFighting, DotRefresh) {
    auto &&[player, target, df] = getTestData();
    df->setExploitedWeakness(true);
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));

    AbilityIds ids{dirty_fighting_shrap_bomb, dirty_fighting_shrap_bomb};

    SetRotation rot(player, ids);
    rot.setTarget(target);
    rot.setStart(Second(0.0));
    rot.setRepeats(1);
    rot.doRotation();
}
namespace {
int countHits(const AbilityId &id, const Target::TargetEvents &events) {
    int count = 0;
    for (auto e : events) {
        if (e.type == Target::TargetEventType::Damage) {
            auto &&hits = *e.damage;
            for (auto &&hit : hits) {
                if (hit.id == id)
                    ++count;
            }
        }
    }
    return count;
}
} // namespace
TEST(DirtyFighting, WeakDots) {
    auto &&[player, target, df] = getTestData();
    df->setExploitedWeakness(true);
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto em = df->getEnergyModel();
    em->setVerbose(true);
    player->setEnergyModel(em);
    AbilityIds ids{
        dirty_fighting_shrap_bomb, smuggler_flurry_of_bolts, smuggler_flurry_of_bolts, smuggler_flurry_of_bolts,
        smuggler_flurry_of_bolts,  smuggler_flurry_of_bolts, smuggler_flurry_of_bolts, smuggler_flurry_of_bolts,
        smuggler_flurry_of_bolts,  smuggler_flurry_of_bolts, smuggler_flurry_of_bolts, smuggler_flurry_of_bolts,
        smuggler_flurry_of_bolts,  smuggler_flurry_of_bolts, smuggler_flurry_of_bolts, smuggler_flurry_of_bolts,
        smuggler_flurry_of_bolts,  smuggler_flurry_of_bolts, smuggler_flurry_of_bolts, dirty_fighting_wounding_shots};

    SetRotation rot(player, ids);
    rot.setClass(df);
    rot.setTarget(target);
    rot.setStart(Second(0.0));
    rot.setRepeats(1);
    rot.doRotation();
}

TEST(DirtyFighting, HitCount) {
    {
        auto &&[player, target, df] = getTestData();
        df->setExploitedWeakness(false);
        addBuffs(player, df->getStaticBuffs(), Second(0.0));
        player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));

        AbilityIds ids{dirty_fighting_hemorraghing_blast};

        SetRotation rot(player, ids);
        rot.setClass(df);
        rot.setTarget(target);
        rot.setStart(Second(0.0));
        rot.setRepeats(1);
        rot.doRotation();
        auto h = countHits(dirty_fighting_hemorraghing_blast, target->getEvents());
        ASSERT_EQ(h, 2);
    }
    {
        auto &&[player, target, df] = getTestData();
        df->setExploitedWeakness(false);
        addBuffs(player, df->getStaticBuffs(), Second(0.0));
        player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));

        AbilityIds ids{dirty_fighting_hemorraghing_blast, dirty_fighting_dirty_blast};

        SetRotation rot(player, ids);
        rot.setClass(df);
        rot.setTarget(target);
        rot.setStart(Second(0.0));
        rot.setRepeats(1);
        rot.doRotation();
        auto h = countHits(dirty_fighting_hemorraghing_blast, target->getEvents());
        ASSERT_EQ(h, 4);
    }
    {
        auto &&[player, target, df] = getTestData();
        df->setExploitedWeakness(false);
        addBuffs(player, df->getStaticBuffs(), Second(0.0));
        player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));

        AbilityIds ids{dirty_fighting_dirty_blast, dirty_fighting_hemorraghing_blast, dirty_fighting_dirty_blast};

        SetRotation rot(player, ids);
        rot.setClass(df);
        rot.setTarget(target);
        rot.setStart(Second(0.0));
        rot.setRepeats(1);
        rot.doRotation();
        auto h = countHits(dirty_fighting_hemorraghing_blast, target->getEvents());
        ASSERT_EQ(h, 6);
    }
}

TEST(DirtyFighting, Instant) {
    auto &&[player, target, df] = getTestDataHA();
    df->setExploitedWeakness(false);
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    // AbilityIds ids{dirty_fighting_dirty_blast,        gunslinger_vital_shot,         dirty_fighting_shrap_bomb,
    //                dirty_fighting_hemorraghing_blast, dirty_fighting_wounding_shots, dirty_fighting_dirty_blast,
    //                dirty_fighting_dirty_blast,        dirty_fighting_dirty_blast,    gunslinger_take_cover,
    //                dirty_fighting_dirty_blast,        dirty_fighting_wounding_shots};
    AbilityIds ids{dirty_fighting_dirty_blast, dirty_fighting_dirty_blast, gunslinger_take_cover,
                   dirty_fighting_dirty_blast};
    SetRotation rot(player, ids);
    rot.setClass(df);
    rot.setTarget(target);
    rot.setStart(Second(0.0));
    rot.setDelayAfterChanneled(Second(0.0));
    rot.setMinTimeAfterInstant(Second(0.0));
    rot.setRepeats(1);
    rot.doRotation();
    auto &&e = target->getEvents();
    logParseInformation(e, getLastDamageEvent(e) - getFirstDamageEvent(e));
}
TEST(DirtyFighting, Clip) {
    {
        auto &&[player, target, df] = getTestDataHA();
        df->setExploitedWeakness(false);
        addBuffs(player, df->getStaticBuffs(), Second(0.0));
        player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
        AbilityIds ids{dirty_fighting_dirty_blast,        gunslinger_vital_shot,         dirty_fighting_shrap_bomb,
                       dirty_fighting_hemorraghing_blast, dirty_fighting_wounding_shots, dirty_fighting_dirty_blast,
                       dirty_fighting_dirty_blast,        dirty_fighting_dirty_blast,    dirty_fighting_dirty_blast,
                       dirty_fighting_wounding_shots};
        // AbilityIds ids{dirty_fighting_dirty_blast, dirty_fighting_wounding_shots, dirty_fighting_dirty_blast};
        SetRotation rot(player, ids);
        rot.setClass(df);
        rot.setTarget(target);
        rot.setStart(Second(-0.1));
        rot.setDelayAfterChanneled(Second(0.0));
        rot.setMinTimeAfterInstant(Second(0.0));
        rot.setRepeats(1);
        rot.doRotation();
        auto &&e = target->getEvents();
        logParseInformation(e, getLastDamageEvent(e) - getFirstDamageEvent(e));
    }
    {
        auto &&[player, target, df] = getTestDataHA();
        df->setExploitedWeakness(false);
        addBuffs(player, df->getStaticBuffs(), Second(0.0));
        player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
        AbilityIds ids{dirty_fighting_dirty_blast,        gunslinger_vital_shot,         dirty_fighting_shrap_bomb,
                       dirty_fighting_hemorraghing_blast, dirty_fighting_wounding_shots, dirty_fighting_dirty_blast,
                       dirty_fighting_dirty_blast,        dirty_fighting_dirty_blast,    dirty_fighting_dirty_blast,
                       dirty_fighting_wounding_shots};
        // AbilityIds ids{dirty_fighting_dirty_blast, dirty_fighting_wounding_shots, dirty_fighting_dirty_blast};
        SetRotation rot(player, ids);
        rot.setClass(df);
        rot.setTarget(target);
        rot.setStart(Second(0.0));
        rot.setDelayAfterChanneled(Second(0.0));
        rot.setMinTimeAfterInstant(Second(0.0));
        rot.setRepeats(1);
        rot.doRotation();
        auto &&e = target->getEvents();
        logParseInformation(e, getLastDamageEvent(e) - getFirstDamageEvent(e));
    }
}