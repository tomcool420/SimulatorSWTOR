#include "../detail/calculations.h"
#include <gtest/gtest.h>

using namespace Simulator;
TEST(calculations, initial) {
    EXPECT_NEAR(0.071438, detail::getAlacrity(AlacrityRating(1213)), 1e-6);
    EXPECT_NEAR(0.153708, detail::getCriticalChance(CriticalRating(2388)), 1e-6);
    EXPECT_NEAR(0.1537083, detail::getCriticalMultiplier(CriticalRating(2388)),1e-6);
    EXPECT_NEAR(2400.12, detail::getBonusDamage(Mastery(12000)), 1e-6);
    EXPECT_NEAR(2760, detail::getBonusDamage(Power(12000)), 1e-6);
    EXPECT_NEAR(2760, detail::getFTBonusDamage(FTPower(12000)), 1e-6);
    EXPECT_NEAR(0.124451, detail::getCriticalChance(Mastery(12000)), 1e-6);
}
