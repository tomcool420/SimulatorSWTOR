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
StatChanges getDefaultStatsBuffs() {
    StatChanges sb;
    sb.masteryMultiplierBonus = 0.07;      // Set bonus + force valor;
    sb.flatCriticalBonus = 0.06;           // companion + lucky shots;
    sb.flatCriticalMultiplierBonus = 0.01; // companion
    sb.bonusDamageMultiplier = 0.05;       // force might
    return sb;
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
std::vector<AbilityBuffPtr> getVanguardSheetBuffs() {
    std::vector<AbilityBuffPtr> ret;
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
        auto hits = calculateDamageRange(stockStrike, stats);
        DamageRange fg{16220, 16784};
        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
        ASSERT_NEAR(fg.first / hits[0].dmg.first, 1.2, 1e-3);
        ASSERT_NEAR(fg.second / hits[0].dmg.second, 1.2, 1e-3);
    }
    {
        Ability tacticalSurge(3393260387041280, 1.72, 0.152, 0.192, 0.0, DamageType::Kinetic, false, false);
        auto hits = calculateDamageRange(tacticalSurge, stats);
        DamageRange fg{14429, 14945};

        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
    }

    Target t;
    auto tacticsBuffs = getVanguardSheetBuffs();
    {
        Ability stockStrike(801367882989568, 1.77, 0.158, 0.198, 0.0, DamageType::Kinetic, false, false);
        auto aStats = stats;
        for (auto &&b : tacticsBuffs) {
            b->apply(stockStrike, aStats, t);
        }
        auto hits = calculateDamageRange(stockStrike, aStats);
        DamageRange fg{16220, 16784};
        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
        ASSERT_NEAR(fg.first / hits[0].dmg.first, 1.2 / 1.27, 1e-3); // High Energy Cell is not in the
        ASSERT_NEAR(fg.second / hits[0].dmg.second, 1.2 / 1.27, 1e-3);
    }
    {
        Ability tacticalSurge(3393260387041280, 1.72, 0.152, 0.192, 0.0, DamageType::Kinetic, false, false);
        auto aStats = stats;
        for (auto &&b : tacticsBuffs) {
            b->apply(tacticalSurge, aStats, t);
        }
        auto hits = calculateDamageRange(tacticalSurge, aStats);
        DamageRange fg{14429, 14945};

        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
        ASSERT_NEAR(fg.first / hits[0].dmg.first, 1.1 / 1.17, 1e-3);
        ASSERT_NEAR(fg.second / hits[0].dmg.second, 1.1 / 1.17, 1e-3);
    }

    {
        Ability highImpactBolt(2021194429628416, 1.97, 0.197, 0.197, 0.31, DamageType::Weapon, false, false);
        auto aStats = stats;
        for (auto &&b : tacticsBuffs) {
            b->apply(highImpactBolt, aStats, t);
        }
        auto hits = calculateDamageRange(highImpactBolt, aStats);
        DamageRange fg{16402, 17607};

        std::cout << fmt::format("Damage for ability with id {} is {} - {}, ratio are {}, {}", hits[0].id,
                                 hits[0].dmg.first, hits[0].dmg.second, fg.first / hits[0].dmg.first,
                                 fg.second / hits[0].dmg.second)
                  << std::endl;
        ASSERT_NEAR(fg.first / hits[0].dmg.first, 1.0, 1e-3);
        ASSERT_NEAR(fg.second / hits[0].dmg.second, 1.0, 1e-3);
    }
}
