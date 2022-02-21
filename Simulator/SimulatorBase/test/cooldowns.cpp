#include "../Target.h"
#include <gtest/gtest.h>

using namespace Simulator;

TEST(Cooldowns, basic) {
    auto t = Target::New({});
    ASSERT_FALSE(t->getNextEventTime());
    t->putAbilityOnCooldown(AbilityId{sharpshooter_aimed_shot}, Second(15), Second(0));
    auto te = t->getNextEventTime();
    ASSERT_TRUE(te);
    EXPECT_NEAR(te->getValue(), 15.0, 1e-3);
    EXPECT_ANY_THROW(t->putAbilityOnCooldown(AbilityId{sharpshooter_aimed_shot}, Second(15), Second(1)));
}