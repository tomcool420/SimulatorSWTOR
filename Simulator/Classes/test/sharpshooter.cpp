#include "../Sharpshooter.h"
#include "Simulator/SimulatorBase/Rotation.h"
#include "Simulator/SimulatorBase/Target.h"
#include "Simulator/SimulatorBase/detail/log.h"
#include "../detail/shared.h"
#include <Simulator/SimulatorBase/parseHelpers.h>
#include <Simulator/SimulatorBase/types.h>
#include <gtest/gtest.h>

using namespace Simulator;
namespace {
struct TestData {
    TargetPtr source;
    TargetPtr target;
    std::shared_ptr<Sharpshooter> df;
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
    ret.df = std::make_shared<Sharpshooter>();
    return ret;
}
DamageRanges getDamageRanges(AbilityId id) {
    detail::LogDisabler d;
    auto &&[player, target, c] = getTestData();
    addBuffs(player, c->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto abl = c->getAbility(id);
    CHECK(abl);
    auto afs = getAllFinalStats(*abl, player, target);
    return calculateDamageRange(*abl, afs);
}

} // namespace

TEST(Sharpshooter, VitalShot) {
    auto DamageRange = getDamageRanges(gunslinger_vital_shot);
    ASSERT_EQ(DamageRange.size(), 1);
    ASSERT_NEAR(std::round(DamageRange[0].dmg.first * 7), 17642, 1.0);
    ASSERT_NEAR(std::round(DamageRange[0].dmg.second * 7), 17642, 1.0);
}

TEST(Sharpshooter, PenetratingBlasts) {
    detail::LogDisabler d;
    auto &&[player, target, df] = getTestData();
    addBuffs(player, df->getStaticBuffs(), Second(0.0));
    player->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
    auto abl = df->getAbility(sharpshooter_penetrating_blasts);
    auto afs = getAllFinalStats(*abl, player, target);
    auto DamageRange = calculateDamageRange(*abl, afs);
    ASSERT_EQ(DamageRange.size(), 2);
    auto info = abl->getInfo();
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first) * info.nTicks), 32146, 1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second) * info.nTicks), 36773, 1.0);
}
TEST(Sharpshooter, AimedShot) {
    auto DamageRange = getDamageRanges(sharpshooter_aimed_shot);
    ASSERT_EQ(DamageRange.size(), 2);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first)), 28071, 1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second)), 31995, 1.0);
}
TEST(Sharpshooter, Quickdraw) {
    auto DamageRange = getDamageRanges(gunslinger_quickdraw);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first)), 19225, 1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second)), 22002, 1.0);
}

TEST(Sharpshooter, ChargedBurst) {
    auto DamageRange = getDamageRanges(gunslinger_charged_burst);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.first + DamageRange[1].dmg.first)), 13998, 1.0);
    ASSERT_NEAR(std::round((DamageRange[0].dmg.second + DamageRange[1].dmg.second)), 16024, 1.0);
}
