#include "../PriorityList.h"
#include "../StaticRotation.h"
#include <Simulator/Classes/DirtyFighting.h>
#include <Simulator/SimulatorBase/constants.h>
#include <Simulator/SimulatorBase/detail/log.h>
#include <Simulator/SimulatorBase/detail/names.h>
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
template <class K, class V> bool contains(const std::map<K, V> &m, const K &k) {
    if (m.find(k) == m.end())
        return false;
    return true;
}
} // namespace
TEST(Complex, test01) {
    auto &&[s, t, c] = getTestData();
    auto p = std::make_shared<PriorityList>();
    p->addAbility(gunslinger_smugglers_luck, {getCooldownFinishedCondition(gunslinger_smugglers_luck)});
    p->addAbility(gunslinger_hunker_down, {getCooldownFinishedCondition(gunslinger_hunker_down)});
    p->addAbility(gunslinger_illegal_mods, {getCooldownFinishedCondition(gunslinger_illegal_mods)});
    auto baseRotation = std::make_shared<StaticRotation>();
    baseRotation->addAbility(gunslinger_vital_shot);
    baseRotation->addAbility(dirty_fighting_shrap_bomb);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_hemorraghing_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_dirty_blast);
    baseRotation->addAbility(dirty_fighting_wounding_shots);
    p->addPriorityList(baseRotation, {});

    DirtyFighting df;
    df.setEstablishedFoothold(true);
    df.setExploitedWeakness(true);

    Second current{0.0};
    std::map<AbilityId, int> counts;
    for (int ii = 0; ii < 80; ++ii) {
        auto na = p->getNextAbility(s, t, current, current);
        if (auto a = std::get_if<AbilityId>(&na)) {
            auto abl = df.getAbility(*a);
            // SIM_INFO("Time: {}, abl: {} - {}", current.getValue(), *a, detail::getAbilityName(*a));
            current += abl->getInfo().type == AbilityCastType::OffGCD ? Second(0.0) : Second(1.5);
            if (abl->getCooldown() > Second(0.0))
                s->setAbilityCooldown(*a, current + abl->getCooldown());
            counts[*a] += 1;
        } else {
            // ASSERT_FALSE(true) << "Did not expect to encounter a delay in this test";
        }
    }
    // for (auto &&abl : counts) {
    //     std::cout << fmt::format("{{{},{}}}, //{}\n", abl.first, abl.second, detail::getAbilityName(abl.first));
    // }
    std::map<AbilityId, int> compare{
        {760484089298944, 1},   // Illegal Mods
        {807698664783872, 8},   // Shrap Bomb
        {807711549685760, 14},  // Wounding Shots
        {962811408678912, 2},   // Smuggler's Luck
        {2044808159821824, 4},  // Hunker Down
        {2115340112756736, 8},  // Vital Shot
        {3404611985604608, 36}, // Dirty Blast
        {3404620575539200, 7}   // Hemorraghing Blast
    };
    ASSERT_EQ(compare.size(), counts.size());
    for (auto &&it : compare) {
        ASSERT_EQ(counts.at(it.first), it.second);
    }
} // namespace