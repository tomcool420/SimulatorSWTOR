#include "../Infiltration.h"
#include "Simulator/SimulatorBase/Rotation.h"
#include <Simulator/SimulatorBase/Ability.h>
#include <Simulator/SimulatorBase/Target.h>
#include <gtest/gtest.h>

using namespace Simulator;
namespace {
struct TestData {
    TargetPtr source;
    TargetPtr target;
    std::shared_ptr<Infiltration> c;
};
TestData getTestData() {
    RawStats rs;
    rs.master = Mastery{2907};
    rs.power = Power{1592};
    rs.accuracyRating = AccuracyRating{539};
    rs.criticalRating = CriticalRating{6000};
    rs.alacrityRating = AlacrityRating{431};
    rs.weaponDamageMH = {1887.0, 2831.0};
    rs.hasOffhand = false;
    rs.forceTechPower = FTPower{7008};
    TestData ret;
    ret.source = Target::New(rs);
    ret.target = Target::New(rs);
    ret.c = std::make_shared<Infiltration>();
    ret.c->setClassBuffs(false);
    return ret;
}

DamageRanges getDamageRanges(AbilityId id) {
    detail::LogDisabler d;
    auto &&[player, target, c] = getTestData();
    addBuffs(player, c->getStaticBuffs(), Second(0.0));
    auto abl = c->getAbility(id);
    CHECK(abl);
    auto afs = getAllFinalStats(*abl, player, target);
    return calculateDamageRange(*abl, afs);
}
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
TEST(DISABLED_Infiltration, abilities) {
    {
        auto r = getDamageRanges(consular_double_strike);
        ASSERT_EQ(r.size(), 2);
        EXPECT_NEAR(std::round(r[0].dmg.first + r[1].dmg.first), 2555 * 2, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second + r[1].dmg.second), 3031 * 2, 1.0);
    }
    {
        auto r = getDamageRanges(shadow_shadow_strike);
        ASSERT_EQ(r.size(), 1);
        EXPECT_NEAR(std::round(r[0].dmg.first), 7869, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second), 9332, 1.0);
    }
    {
        auto r = getDamageRanges(shadow_spinning_strike);
        ASSERT_EQ(r.size(), 1);
        EXPECT_NEAR(std::round(r[0].dmg.first), 8598, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second), 10203, 1.0);
    }
    {
        auto r = getDamageRanges(consular_saber_strike);
        ASSERT_EQ(r.size(), 2);
        EXPECT_NEAR(std::round(r[0].dmg.first + r[1].dmg.first), 2844, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second + r[1].dmg.second), 3797, 1.0);
    }
    {
        auto r = getDamageRanges(infiltration_clairvoyant_strike);
        ASSERT_EQ(r.size(), 2);
        EXPECT_NEAR(std::round(r[0].dmg.first + r[1].dmg.first), 3169 * 2, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second + r[1].dmg.second), 3759 * 2, 1.0);
    }
    {
        auto r = getDamageRanges(infiltration_vaulting_slash);
        ASSERT_EQ(r.size(), 1);
        EXPECT_NEAR(std::round(r[0].dmg.first), 11147, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second), 13220, 1.0);
    }
    {
        auto r = getDamageRanges(infiltration_psychokinetic_blast);
        ASSERT_EQ(r.size(), 1);
        EXPECT_NEAR(std::round(r[0].dmg.first), 6635, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second), 7105, 1.0);
    }
    {
        auto r = getDamageRanges(infiltration_force_breach);
        ASSERT_EQ(r.size(), 1);
        EXPECT_NEAR(std::round(r[0].dmg.first), 2491, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second), 2961, 1.0);
    }
    {
        auto &&[player, target, c] = getTestData();
        addBuffs(player, c->getStaticBuffs(), Second(0.0));
        auto abl = c->getAbility(infiltration_force_breach);
        CHECK(abl);
        auto b = player->getBuff<Buff>(infiltration_shadow_technique);
        b->setCurrentStacks(3, Second(0));

        auto afs = getAllFinalStats(*abl, player, target);
        auto r = calculateDamageRange(*abl, afs);
        ASSERT_EQ(r.size(), 1);
        EXPECT_NEAR(std::round(r[0].dmg.first), 2491 * 3, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second), 2961 * 3, 1.0);
    }
    {
        auto &&[player, target, c] = getTestData();
        addBuffs(player, c->getStaticBuffs(), Second(0.0));
        auto abl = c->getAbility(shadow_shadow_strike);
        CHECK(abl);
        auto b = player->getBuff<Buff>(infiltration_infiltration_tactics);
        b->setCurrentStacks(1, Second(0));

        auto afs = getAllFinalStats(*abl, player, target);
        auto r = calculateDamageRange(*abl, afs);
        ASSERT_EQ(r.size(), 1);
        EXPECT_NEAR(std::round(r[0].dmg.first), 9443, 1.0);
        EXPECT_NEAR(std::round(r[0].dmg.second), 11199, 1.0);
    }
}
TEST(Infiltration, BasicTest) {
    auto &&[player, target, c] = getTestData();
    addBuffs(player, c->getStaticBuffs(), Second(0.0));
    AbilityIds ids{infiltration_clairvoyant_strike,
                   infiltration_clairvoyant_strike,
                   infiltration_vaulting_slash,
                   infiltration_psychokinetic_blast,
                   infiltration_psychokinetic_blast,
                   infiltration_force_breach,
                   shadow_shadow_strike,
                   shadow_shadow_strike};
    SetRotation rot(player, ids);
    rot.setClass(c);
    rot.setTarget(target);
    rot.setStart(Second(0.0));
    rot.setRepeats(1);
    rot.setContinueEvenIfOnlySource(true);
    rot.doRotation();
    auto h = countHits(infiltration_clairvoyant_strike, target->getEvents());
    ASSERT_EQ(h, 4);
}