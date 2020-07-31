#include "../../Classes/DirtyFighting.h"
#include "../../Classes/detail/shared.h"
#include "../../Rotations/Condition.h"
#include "../../Rotations/PriorityList.h"
#include "../../libSimulator/Buff.h"
#include "../../libSimulator/Debuff.h"
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

TEST(Condition, Cooldown) {
    auto [s, t, c] = getTestData();
    CooldownCondition cc(gunslinger_hunker_down);
    ASSERT_TRUE(cc.check(s, t, Second(0.0), Second(0.0)));
    s->setAbilityCooldown(gunslinger_hunker_down, Second(35));
    ASSERT_FALSE(cc.check(s, t, Second(0.0), Second(10.0)));
    ASSERT_TRUE(cc.check(s, t, Second(0.0), Second(35.0)));
}

TEST(Condition, Energy) {
    {
        auto [s, t, c] = getTestData();
        EnergyCondition cc(70);
        ASSERT_TRUE(cc.check(s, t, Second(0.0), Second(0.0)));
        s->setEnergyModel(c->getEnergyModel());
        ASSERT_TRUE(cc.check(s, t, Second(0.0), Second(0.0)));
        s->spendEnergy(45, Second(0.0));
        ASSERT_FALSE(cc.check(s, t, Second(0.0), Second(0.0)));
        s->addEnergy(30, Second(0.0));
        ASSERT_TRUE(cc.check(s, t, Second(0.0), Second(0.0)));
    }
    {
        auto [s, t, c] = getTestData();
        EnergyCondition cc(70, false);
        ASSERT_TRUE(!cc.check(s, t, Second(0.0), Second(0.0)));
        s->setEnergyModel(c->getEnergyModel());
        ASSERT_TRUE(!cc.check(s, t, Second(0.0), Second(0.0)));
        s->spendEnergy(45, Second(0.0));
        ASSERT_FALSE(!cc.check(s, t, Second(0.0), Second(0.0)));
        s->addEnergy(30, Second(0.0));
        ASSERT_TRUE(!cc.check(s, t, Second(0.0), Second(0.0)));
    }
}

TEST(Condition, SubThirtyCondition) {
    {
        auto [s, t, c] = getTestData();
        SubThirtyCondition cc;
        ASSERT_FALSE(cc.check(s, t, Second(0.0), Second(0.0)));
        t->setCurrentHealth(t->getMaxHealth() * 0.2999);
        ASSERT_TRUE(cc.check(s, t, Second(0.0), Second(0.0)));
    }
}

TEST(Condition, Buff) {
    {
        auto [s, t, c] = getTestData();
        auto b = std::make_unique<Buff>();
        b->setId(gunslinger_hunker_down);
        b->setDuration(Second{10});
        BuffCondition cc(gunslinger_hunker_down, Second(0), false);
        ASSERT_FALSE(cc.check(s, t, Second(0.0), Second(0.0)));
        s->addBuff(std::move(b), Second(0.0));
        ASSERT_TRUE(cc.check(s, t, Second(0.0), Second(0.0)));
    }
}

TEST(Condition, Debuff) {
    {
        auto [s, t, c] = getTestData();
        auto b = std::make_unique<Debuff>(gunslinger_hunker_down);
        b->setDuration(Second{10});
        DebuffCondition cc(gunslinger_hunker_down, Second(0), false);
        ASSERT_FALSE(cc.check(s, t, Second(0.0), Second(0.0)));
        t->addDebuff(std::move(b), s, Second(0.0));
        ASSERT_TRUE(cc.check(s, t, Second(0.0), Second(0.0)));
        ASSERT_FALSE(cc.check(s, t, Second(0.0), Second(10.1)));
    }
}

TEST(Condition, SerializationRoundtrip) {
    Conditions c;
    c.push_back(std::make_unique<DebuffCondition>(gunslinger_hunker_down, Second(0.0)));
    c.push_back(std::make_unique<BuffCondition>(gunslinger_hunker_down, Second(0.0)));
    c.push_back(std::make_unique<EnergyCondition>(30));
    c.push_back(std::make_unique<SubThirtyCondition>());
    c.push_back(std::make_unique<CooldownCondition>(gunslinger_hunker_down));

    auto j = serializeConditions(c);
    Conditions cc = deserializeConditions(j);

    ASSERT_EQ(cc.size(), 5);
    ASSERT_TRUE(dynamic_cast<DebuffCondition *>(cc[0].get()));
    ASSERT_FALSE(dynamic_cast<DebuffCondition *>(cc[1].get()));
    ASSERT_TRUE(dynamic_cast<BuffCondition *>(cc[1].get()));
    ASSERT_TRUE(dynamic_cast<EnergyCondition *>(cc[2].get()));
    ASSERT_TRUE(dynamic_cast<SubThirtyCondition *>(cc[3].get()));
    ASSERT_TRUE(dynamic_cast<CooldownCondition *>(cc[4].get()));
}