#include "../StatBuff.h"
#include "../detail/calculations.h"
#include <gtest/gtest.h>

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
    StatChanges sb;
    sb.masteryMultiplierBonus = 0.07;      // Set bonus + force valor;
    sb.flatCriticalBonus = 0.06;           // companion + lucky shots;
    sb.flatCriticalMultiplierBonus = 0.01; // companion
    sb.bonusDamageMultiplier = 0.05;       // force might

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
