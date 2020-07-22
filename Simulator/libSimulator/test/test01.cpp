#include "helpers.h"
#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../StatBuff.h"
#include "../detail/calculations.h"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

using namespace Simulator;
TEST(calculations, initial) {
    EXPECT_NEAR(0.071438, detail::getAlacrity(AlacrityRating(1213)), 1e-6);
    EXPECT_NEAR(0.153708, detail::getCriticalChance(CriticalRating(2388)), 1e-6);
    EXPECT_NEAR(0.1537083, detail::getCriticalMultiplier(CriticalRating(2388)), 1e-6);
    EXPECT_NEAR(2400.12, detail::getBonusDamage(Mastery(12000)), 1e-6);
    EXPECT_NEAR(2760, detail::getBonusDamage(Power(12000)), 1e-6);
    EXPECT_NEAR(2760, detail::getFTBonusDamage(FTPower(12000)), 1e-6);
    EXPECT_NEAR(0.124451, detail::getCriticalChance(Mastery(12000)), 1e-6);
}

TEST(StatBuff, basic) {
    StatChanges forceValorChanges{};
    forceValorChanges.masteryMultiplierBonus = 0.05;
    StatBuff forceValor(forceValorChanges);

    StatChanges twoPieceChanges{};
    twoPieceChanges.masteryMultiplierBonus = 0.02;
    StatBuff twoPiece(twoPieceChanges);

    StatChanges sum;
    forceValor.apply(sum);
    ASSERT_DOUBLE_EQ(sum.masteryMultiplierBonus, 0.05);
    twoPiece.apply(sum);
    ASSERT_DOUBLE_EQ(sum.masteryMultiplierBonus, 0.07);
}

TEST(StatBuff, apply) {
    StatChanges sb = getDefaultStatsBuffs();

    RawStats rs;
    rs.master = Mastery(11344);
    rs.power = Power(10049);
    rs.criticalRating = CriticalRating(2095);
    rs.alacrityRating = AlacrityRating(2331);
    rs.accuracyRating = AccuracyRating(1592);
    rs.forceTechPower = FTPower(7008);

    auto stats = getFinalStats(rs, sb);
    EXPECT_NEAR(stats.forceTechBonusDamage, 6668.3897, 1e-3);
    EXPECT_NEAR(stats.forceTechCritChance, 0.37552, 1e-5);
    EXPECT_NEAR(stats.forceTechCritMultiplier, 0.65023, 1e-5);
    EXPECT_NEAR(stats.meleeRangeBonusDamage, 4975.9577, 1e-3);
    EXPECT_NEAR(stats.meleeRangeCritChance, 0.37552, 1e-5);
    EXPECT_NEAR(stats.meleeRangeCritMultiplier, 0.65023, 1e-5);
    EXPECT_NEAR(stats.alacrity, 0.122116, 1e-5);
}
std::vector<BuffPtr> getVanguardSheetBuffs() {
    std::vector<BuffPtr> ret;
    ret.push_back(std::make_unique<RawSheetBuff>("High Friction Bolts",
                                                 std::vector<uint64_t>{
                                                     2021194429628416 // high Impact Bolt
                                                 },
                                                 0.0, 0.0, 0.0, 0.3));
    ret.push_back(std::make_unique<RawSheetBuff>("Riot Augs",
                                                 std::vector<uint64_t>{
                                                     801367882989568,  // stockstrike
                                                     2021194429628416, // high Impact Bolt
                                                     3393260387041280, // tactical surge
                                                     3393354876321792, // cell burst
                                                     3393277566910464, // assault plastique
                                                 },
                                                 0.1, 0.0, 0.0, 0.0));

    ret.push_back(std::make_unique<RawSheetBuff>("Havoc Training",
                                                 std::vector<uint64_t>{
                                                     801367882989568,  // stockstrike
                                                     2021194429628416, // high Impact Bolt
                                                     3393260387041280, // tactical surge
                                                     3393354876321792, // cell burst
                                                     3393277566910464, // assault plastique
                                                 },
                                                 0.0, 0.0, 0.1, 0.0));

    ret.push_back(std::make_unique<RawSheetBuff>("Focused Impact",
                                                 std::vector<uint64_t>{
                                                     2021194429628416, // high Impact Bolt
                                                 },
                                                 0.0, 0.0, 0.0, 0.6));

    ret.push_back(std::make_unique<RawSheetBuff>("Focused Impact",
                                                 std::vector<uint64_t>{
                                                     801367882989568, // stockstrike
                                                 },
                                                 0.1, 0.0, 0.0, 0.0));

    ret.push_back(std::make_unique<DamageTypeBuff>("High Energy Gas Cell",
                                                   std::vector<DamageType>{DamageType::Kinetic, // stockstrike
                                                                           DamageType::Energy,  // stockstrike
                                                                           DamageType::Weapon},
                                                   0.07, 0.0, 0.0, 0.0));

    return ret;
}
TEST(AbilityDamageCalculation, Tech) {

    auto sb = getDefaultStatsBuffs();
    RawStats rs;
    rs.master = Mastery(11466);
    rs.power = Power(9604);
    rs.criticalRating = CriticalRating(3265);
    rs.alacrityRating = AlacrityRating(1293);
    rs.accuracyRating = AccuracyRating(1652);
    rs.forceTechPower = FTPower(7008);
    rs.weaponDamageMH = {1573.0, 2359.0};
    auto stats = getFinalStats(rs, sb);
    {
        Ability stockStrike(801367882989568, 1.77, 0.158, 0.198, 0.0, DamageType::Kinetic, false, false);
        auto hits = calculateDamageRange(stockStrike, {stats});
        std::pair<double, double> fg{16220, 16784};
        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
        ASSERT_NEAR(fg.first / hits[0].dmg.first, 1.2, 1e-3);
        ASSERT_NEAR(fg.second / hits[0].dmg.second, 1.2, 1e-3);
    }
    {
        Ability tacticalSurge(3393260387041280, 1.72, 0.152, 0.192, 0.0, DamageType::Kinetic, false, false);
        auto hits = calculateDamageRange(tacticalSurge, {stats});
        std::pair<double, double> fg{14429, 14945};

        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
    }

    TargetPtr t = Target::New(rs);
    auto tacticsBuffs = getVanguardSheetBuffs();
    {
        Ability stockStrike(801367882989568, 1.77, 0.158, 0.198, 0.0, DamageType::Kinetic, false, false);

        auto sc = sb;
        AllStatChanges asc{sc};
        for (auto &&b : tacticsBuffs) {
            b->apply(stockStrike, asc, t);
        }
        auto aStats = getFinalStats(rs, asc[0]);

        auto hits = calculateDamageRange(stockStrike, {aStats});
        std::pair<double, double> fg{16220, 16784};
        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
        ASSERT_NEAR(fg.first / hits[0].dmg.first, 1.2 / 1.27, 1e-3); // High Energy Cell is not in the
        ASSERT_NEAR(fg.second / hits[0].dmg.second, 1.2 / 1.27, 1e-3);
    }
    {
        Ability tacticalSurge(3393260387041280, 1.72, 0.152, 0.192, 0.0, DamageType::Kinetic, false, false);
        auto sc = sb;
        AllStatChanges asc{sc};
        for (auto &&b : tacticsBuffs) {
            b->apply(tacticalSurge, asc, t);
        }
        auto aStats = getFinalStats(rs, asc[0]);
        auto hits = calculateDamageRange(tacticalSurge, {aStats});
        std::pair<double, double> fg{14429, 14945};

        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
        ASSERT_NEAR(fg.first / hits[0].dmg.first, 1.1 / 1.17, 1e-3);
        ASSERT_NEAR(fg.second / hits[0].dmg.second, 1.1 / 1.17, 1e-3);
    }

    {
        Ability highImpactBolt(2021194429628416, 1.97, 0.197, 0.197, 0.31, DamageType::Weapon, false, false,false);
        auto sc = sb;
        AllStatChanges asc{sc};
        for (auto &&b : tacticsBuffs) {
            b->apply(highImpactBolt, asc, t);
        }
        auto aStats = getFinalStats(rs, asc[0]);
        auto hits = calculateDamageRange(highImpactBolt, {aStats});
        std::pair<double, double> fg{16402, 17607};

        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
        ASSERT_NEAR(fg.first / hits[0].dmg.first, 1.0, 1e-3);
        ASSERT_NEAR(fg.second / hits[0].dmg.second, 1.0, 1e-3);
    }
}

TEST(AbilityDamageCalculation, VitalShot) {
    RawStats rs;
    rs.master = Mastery(3063);
    rs.power = Power(1304);
    rs.criticalRating = CriticalRating(903);
    rs.alacrityRating = AlacrityRating(0);
    rs.accuracyRating = AccuracyRating(0);
    rs.forceTechPower = FTPower(7008);
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {1376.0, 2556.0};
    rs.hasOffhand = true;
    auto t = Target::New(rs); // create a dummy with 1 mil HP;

    auto statBuffs = getDefaultStatsBuffPtr(false, false);
    auto player = Target::New(rs);
    auto vitalShot = getDot(gunslinger_vital_shot);
    vitalShot->setSource(player);
    StatChanges sc;
    AllStatChanges asc{sc};

    statBuffs->apply(vitalShot->getAbility(), asc, t);

    auto gStats = getFinalStats(rs, asc[0]);

    auto damageRange = calculateDamageRange(vitalShot->getAbility(), {gStats});
    for (auto &&dr : damageRange) {
        std::cout << fmt::format("Damage range for vital shot is : {}-{}", dr.dmg.first, dr.dmg.second) << std::endl;
        ASSERT_NEAR(dr.dmg.first, 1176.221, 1e-3);
    }
    auto shrapBomb = getDot(dirty_fighting_shrap_bomb);
    gStats = getFinalStats(rs, asc[0]);
    auto damageRangeSB = calculateDamageRange(shrapBomb->getAbility(), {gStats});
    for (auto &&dr : damageRangeSB) {
        std::cout << fmt::format("Damage range for shrapBomb is : {}-{}", dr.dmg.first, dr.dmg.second) << std::endl;
        ASSERT_NEAR(dr.dmg.first, 1084.41873, 1e-3);
        ASSERT_NEAR(dr.dmg.first / 1.05 * 1.12, 1156.7133, 1e-3);
    }
}
