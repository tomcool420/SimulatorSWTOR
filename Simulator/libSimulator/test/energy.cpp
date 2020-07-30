#include "../Energy.h"
#include "../Target.h"
#include <gtest/gtest.h>
using namespace Simulator;

TEST(Energy, Knight) {
    Energy e;
    e.setMaxEnergy(12);
    e.setCurrentEnergy(0);
    ASSERT_FALSE(e.getNextEventTime());
    e.addEnergy(6, Second(0.0));
    ASSERT_NEAR(e.getCurrentEnergy(), 6, 1e-3);
    ASSERT_FALSE(e.getNextEventTime());
    ASSERT_ANY_THROW(e.spendEnergy(7, Second(0.0)));
    ASSERT_NO_THROW(e.spendEnergy(3, Second(0.0)));
    ASSERT_EQ(e.getCurrentEnergy(), 3);
    ASSERT_FALSE(e.getNextEventTime());
}

TEST(Energy, Gunslinger) {
    auto t = Target::New({});
    Energy e;
    e.setMaxEnergy(110);
    e.setCurrentEnergy(110);
    e.addEnergyThreshold(66, 5);
    e.addEnergyThreshold(22, 3);
    e.addEnergyThreshold(0, 2);
    e.setEnergyTickRate(Second(1.0));
    ASSERT_FALSE(e.getNextEventTime());
    e.addEnergy(6, Second(0.0));
    ASSERT_NEAR(e.getCurrentEnergy(), 110, 1e-3);
    e.spendEnergy(30, Second(0.0));
    ASSERT_TRUE(e.getNextEventTime());
    ASSERT_NEAR(e.getNextEventTime().value().getValue(), Second(1.0).getValue(), Second(1e-3).getValue());
    e.resolveEventsUpToTime(e.getNextEventTime().value(), t);
    ASSERT_NEAR(e.getCurrentEnergy(), 85, 1e-3);
    ASSERT_NEAR(e.getNextEventTime().value().getValue(), 2.0, 1e-3);
    e.spendEnergy(20, Second(1.5));
    ASSERT_NEAR(e.getCurrentEnergy(), 65, 1e-3);

    e.resolveEventsUpToTime(e.getNextEventTime().value(), t);
    ASSERT_NEAR(e.getCurrentEnergy(), 68, 1e-3);
    e.spendEnergy(50, Second(2.5));
    ASSERT_NEAR(e.getCurrentEnergy(), 18, 1e-3);
    e.resolveEventsUpToTime(e.getNextEventTime().value(), t);
    ASSERT_NEAR(e.getCurrentEnergy(), 20, 1e-3);
}

TEST(Energy, Source) {
    RawStats rs;
    auto t = Target::New(rs);
    auto e = std::make_shared<Energy>();
    e->setMaxEnergy(110);
    e->setCurrentEnergy(110);
    e->addEnergyThreshold(66, 5);
    e->addEnergyThreshold(22, 3);
    e->addEnergyThreshold(0, 2);
    e->setEnergyTickRate(Second(1.0));
    ASSERT_FALSE(t->getEnergyModel());
    t->setEnergyModel(e);
    ASSERT_FALSE(t->getNextEventTime());
    t->spendEnergy(15, Second(0.0));
    auto ne = t->getNextEventTime();
    ASSERT_TRUE(ne);
    ASSERT_NEAR(ne.value().getValue(), 1.0, 1e-3);
    t->applyEventsAtTime(*ne);
    ASSERT_EQ(t->getEvents().size(), 2);
    ne = t->getNextEventTime();
    t->applyEventsAtTime(*ne);
    ASSERT_EQ(t->getEvents().size(), 3);
    ASSERT_NEAR(t->getEnergyModel()->getCurrentEnergy(), 105, 1e-3);
}