#include "../../Classes/DirtyFighting.h"
#include "../StaticRotation.h"
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
    rs.master = Mastery{4953};
    rs.power = Power{2100};
    rs.accuracyRating = AccuracyRating{1557};
    rs.criticalRating = CriticalRating{313};
    rs.alacrityRating = AlacrityRating{0};
    rs.weaponDamageMH = {1376.0, 2556.0};
    rs.weaponDamageOH = {1376.0 * 0.3, 2556.0 * 0.3};
    rs.forceTechPower = FTPower{7008};
    TestData ret;
    ret.source = Target::New(rs);
    ret.target = Target::New(rs);
    ret.df = std::make_shared<DirtyFighting>();
    return ret;
}
}
TEST(StaticRotation, Simple) {
    auto &&[s, t, c] = getTestData();
    StaticRotation r;
    r.addAbility(gunslinger_speed_shot);
    r.addAbility(gunslinger_vital_shot);
    r.addAbility(dirty_fighting_shrap_bomb);
    ASSERT_EQ(r.getSize(), 3);
    auto a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    ASSERT_EQ(a, gunslinger_speed_shot);
    ASSERT_EQ(r.getIndex(), 1);
    a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    ASSERT_EQ(a, dirty_fighting_shrap_bomb);
    ASSERT_EQ(r.getIndex(), 3);
    auto av = r.getNextAbility(s, t, Second(0.0), Second(0.0));
    ASSERT_ANY_THROW(static_cast<void>( std::get<Second>(av)));
    ASSERT_EQ(std::get<AbilityId>(av), gunslinger_speed_shot);
    r.log(std::cout, 0);
}