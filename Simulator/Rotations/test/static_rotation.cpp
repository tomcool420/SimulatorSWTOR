#include "../../Classes/DirtyFighting.h"
#include "../PriorityList.h"
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
} // namespace
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
    ASSERT_ANY_THROW(static_cast<void>(std::get<Second>(av)));
    ASSERT_EQ(std::get<AbilityId>(av), gunslinger_speed_shot);
    ASSERT_NO_THROW(r.log(std::cout, 0));
}

TEST(StaticRotation, Delay) {
    auto &&[s, t, c] = getTestData();
    StaticRotation r;
    r.addAbility(gunslinger_speed_shot);
    r.addAbility(gunslinger_vital_shot);
    r.addAbility(dirty_fighting_shrap_bomb);
    r.addDelay(Second(3));
    ASSERT_EQ(r.getSize(), 4);
    auto a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    ASSERT_EQ(a, gunslinger_speed_shot);
    ASSERT_EQ(r.getIndex(), 1);
    a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    ASSERT_EQ(a, dirty_fighting_shrap_bomb);
    auto av = r.getNextAbility(s, t, Second(0.0), Second(0.0));
    ASSERT_TRUE(std::holds_alternative<Second>(av));
    ASSERT_EQ(r.getIndex(), 4);
    av = r.getNextAbility(s, t, Second(0.0), Second(0.0));
    ASSERT_ANY_THROW(static_cast<void>(std::get<Second>(av)));
    ASSERT_EQ(std::get<AbilityId>(av), gunslinger_speed_shot);
    ASSERT_NO_THROW(r.log(std::cout, 0));
}

TEST(StaticRotation, PriorityList) {
    auto &&[s, t, c] = getTestData();
    StaticRotation r;
    r.addAbility(gunslinger_speed_shot);
    r.addAbility(gunslinger_vital_shot);
    auto p = std::make_shared<PriorityList>();
    p->addAbility(dirty_fighting_dirty_blast, {});
    r.addPriorityList(p);
    r.addAbility(dirty_fighting_shrap_bomb);
    ASSERT_EQ(r.getSize(), 4);
    auto a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    ASSERT_EQ(a, gunslinger_speed_shot);
    a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    a = std::get<AbilityId>(r.getNextAbility(s, t, Second(0.0), Second(0.0)));
    ASSERT_EQ(a, dirty_fighting_dirty_blast);
    ASSERT_NO_THROW(r.log(std::cout, 0));
}

TEST(StaticRotation, serialization) {
    StaticRotation r;
    r.addAbility(gunslinger_speed_shot);
    r.addAbility(gunslinger_speed_shot);
    r.addAbility(gunslinger_speed_shot);
    r.addAbility(gunslinger_speed_shot);
    r.addAbility(gunslinger_vital_shot);
    r.addAbility(dirty_fighting_shrap_bomb);
    r.addDelay(Second(3));
    auto p = std::make_shared<PriorityList>();
    p->addAbility(gunslinger_smugglers_luck, {getCooldownFinishedCondition(gunslinger_smugglers_luck)});
    p->addAbility(gunslinger_hunker_down, {getCooldownFinishedCondition(gunslinger_hunker_down)});
    r.addPriorityList(p);
    auto serialized = r.serialize();
    std::cout << std::setw(3) << serialized << std::endl;
}
TEST(Combined, SerializationRoundtrip) {
    auto p = std::make_shared<PriorityList>();
    p->addAbility(gunslinger_smugglers_luck, getCooldownFinishedCondition(gunslinger_smugglers_luck));
    p->addAbility(gunslinger_hunker_down, getCooldownFinishedCondition(gunslinger_hunker_down));
    p->addAbility(gunslinger_illegal_mods, getCooldownFinishedCondition(gunslinger_illegal_mods));

    auto baseRotation = std::make_shared<StaticRotation>();
    baseRotation->addAbility(gunslinger_vital_shot);
    baseRotation->addAbility(dirty_fighting_shrap_bomb);
    auto p2 = std::make_shared<PriorityList>();
    Conditions c;
    c.push_back(std::make_unique<SubThirtyCondition>());
    p2->addAbility(dirty_fighting_dirty_blast, std::move(c));
    p2->addAbility(gunslinger_quickdraw, {});
    baseRotation->addPriorityList(p2);
    baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);
    p->addPriorityList(baseRotation, {});
    auto j_orig = p->serialize();

    auto pds = RotationalPriorityList::deserialize(j_orig);

    auto j_rt = pds->serialize();

    std::stringstream ss0;
    ss0 << j_orig;
    auto ss0str = ss0.str();

    std::stringstream ss1;
    ss1 << j_rt;
    auto ss1str = ss1.str();
    ASSERT_EQ(ss0str, ss1str);
}