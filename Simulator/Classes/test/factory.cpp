#include "../factory.h"
#include "../DirtyFighting.h"
#include <gtest/gtest.h>
using namespace Simulator;

TEST(Factory, Fail) {
    nlohmann::json j;
    ASSERT_ANY_THROW(Simulator::createClassFromJson(j));
    j[key_class] = "FakeDirtyFighting";
    ASSERT_ANY_THROW(Simulator::createClassFromJson(j));
    j[key_class] = key_class_dirty_fighting;
    ASSERT_NO_THROW(Simulator::createClassFromJson(j));
}

TEST(Factory, DirtyFighting) {
    nlohmann::json j;
    j[key_class] = key_class_dirty_fighting;
    {
        auto c = Simulator::createClassFromJson(j);
        ASSERT_TRUE(dynamic_cast<DirtyFighting *>(c.get()));
        auto df = dynamic_cast<DirtyFighting *>(c.get());
        ASSERT_FALSE(df->getEstablishedFoothold());
        ASSERT_FALSE(df->getExploitedWeakness());
        ASSERT_TRUE(df->getLayLowPassive());
    }
    j[key_lay_low] = true;
    {
        auto c = Simulator::createClassFromJson(j);
        ASSERT_TRUE(dynamic_cast<DirtyFighting *>(c.get()));
        auto df = dynamic_cast<DirtyFighting *>(c.get());
        ASSERT_FALSE(df->getEstablishedFoothold());
        ASSERT_FALSE(df->getExploitedWeakness());
        ASSERT_TRUE(df->getLayLowPassive());
    }
    j[key_lay_low] = false;
    {
        auto c = Simulator::createClassFromJson(j);
        ASSERT_TRUE(dynamic_cast<DirtyFighting *>(c.get()));
        auto df = dynamic_cast<DirtyFighting *>(c.get());
        ASSERT_FALSE(df->getEstablishedFoothold());
        ASSERT_FALSE(df->getExploitedWeakness());
        ASSERT_FALSE(df->getLayLowPassive());
    }
    j[key_exploited_weakness] = true;
    {
        auto c = Simulator::createClassFromJson(j);
        ASSERT_TRUE(dynamic_cast<DirtyFighting *>(c.get()));
        auto df = dynamic_cast<DirtyFighting *>(c.get());
        ASSERT_FALSE(df->getEstablishedFoothold());
        ASSERT_TRUE(df->getExploitedWeakness());
        ASSERT_FALSE(df->getLayLowPassive());
    }
    j[key_established_foothold] = true;
    {
        auto c = Simulator::createClassFromJson(j);
        ASSERT_TRUE(dynamic_cast<DirtyFighting *>(c.get()));
        auto df = dynamic_cast<DirtyFighting *>(c.get());
        ASSERT_TRUE(df->getEstablishedFoothold());
        ASSERT_TRUE(df->getExploitedWeakness());
        ASSERT_FALSE(df->getLayLowPassive());
    }
}

TEST(Serialization, DirtyFighting) {
    nlohmann::json j;
    j[key_class] = key_class_dirty_fighting;
    j[key_established_foothold] = true;
    j[key_exploited_weakness] = true;
    j[key_lay_low] = false;

    auto c = Simulator::createClassFromJson(j);
    ASSERT_TRUE(dynamic_cast<DirtyFighting *>(c.get()));
    auto df = dynamic_cast<DirtyFighting *>(c.get());
    ASSERT_TRUE(df->getEstablishedFoothold());
    ASSERT_TRUE(df->getExploitedWeakness());
    ASSERT_FALSE(df->getLayLowPassive());

    auto jo = c->serialize();

    ASSERT_EQ(key_class_dirty_fighting, jo[key_class].get<std::string>());
    ASSERT_TRUE(jo[key_exploited_weakness].get<bool>());
    ASSERT_TRUE(jo[key_established_foothold].get<bool>());
    ASSERT_FALSE(jo[key_lay_low].get<bool>());
}