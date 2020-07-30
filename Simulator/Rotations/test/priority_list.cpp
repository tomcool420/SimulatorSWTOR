#include "../../Classes/DirtyFighting.h"
#include "../../Classes/detail/shared.h"
#include "../../Rotations/Condition.h"
#include "../../Rotations/PriorityList.h"
#include "../../libSimulator/Rotation.h"
#include "../../libSimulator/Target.h"
#include "../../libSimulator/detail/log.h"
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
} // namespace
TEST(PriorityList, Simple) {
    PriorityList p;
    auto &&[s, t, c] = getTestData();

    p.addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
    p.addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
    p.addAbility(dirty_fighting_dirty_blast, {});

    auto rr = std::get<AbilityId>(p.getNextAbility(s, t, Second(0.0), Second(0.0)));
    ASSERT_EQ(rr, gunslinger_smugglers_luck);
    s->setAbilityCooldown(gunslinger_smugglers_luck, Second(60));
    rr = std::get<AbilityId>(p.getNextAbility(s, t, Second(0.1), Second(0.1)));
    ASSERT_EQ(rr, gunslinger_hunker_down);
    s->setAbilityCooldown(gunslinger_hunker_down, Second(35.1));
    rr = std::get<AbilityId>(p.getNextAbility(s, t, Second(0.2), Second(0.2)));
    ASSERT_EQ(rr, dirty_fighting_dirty_blast);
    rr = std::get<AbilityId>(p.getNextAbility(s, t, Second(1.7), Second(1.7)));
    ASSERT_EQ(rr, dirty_fighting_dirty_blast);
    rr = std::get<AbilityId>(p.getNextAbility(s, t, Second(35), Second(35)));
    ASSERT_EQ(rr, dirty_fighting_dirty_blast);
    rr = std::get<AbilityId>(p.getNextAbility(s, t, Second(35.1), Second(35.1)));
    ASSERT_EQ(rr, gunslinger_hunker_down);
    s->setAbilityCooldown(gunslinger_hunker_down, Second(70.2));
    rr = std::get<AbilityId>(p.getNextAbility(s, t, Second(60), Second(60.0)));
    ASSERT_EQ(rr, gunslinger_smugglers_luck);
}

TEST(PriorityList, serialization) {
    PriorityList p;
    p.addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
    p.addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
    p.addAbility(dirty_fighting_dirty_blast, {});
    auto serialized = p.serialize();
    std::cout << std::setw(3) << serialized << std::endl;
}
