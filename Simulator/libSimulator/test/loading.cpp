#include "../run.h"
#include <Simulator/Classes/DirtyFighting.h>
#include <Simulator/Classes/factory.h>
#include <Simulator/SimulatorBase/Target.h>
#include <Simulator/SimulatorBase/types.h>
#include <filesystem>
#include <gtest/gtest.h>
using namespace Simulator;

namespace {
struct TestData {
    TargetPtr source;
    TargetPtr target;
    std::shared_ptr<DirtyFighting> df;
    RawStats rs;
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
    ret.rs = rs;
    return ret;
}
template <class K, class V> bool contains(const std::map<K, V> &m, const K &k) {
    if (m.find(k) == m.end())
        return false;
    return true;
}
} // namespace

TEST(Loading, JSON) {
    auto &&[s, t, c, rs] = getTestData();
    auto jc = c->serialize();
    auto c2 = createClassFromJson(jc);
    auto jc2 = c2->serialize();
    ASSERT_EQ(jc2, jc);
    nlohmann::json jrs = rs;
    std::cout << jrs << std::endl;
}

TEST(Saving, Stats) {
    auto &&[s, t, c, rs] = getTestData();
    Amplifiers amp;
    amp.periodicIntensity = 0.2068;
    nlohmann::json gj;
    gj[key_class] = c->serialize();
    gj[key_base_stats] = rs;
    gj[key_amplifiers] = amp;
    std::cout << std::setw(1) << gj << std::setw(0) << std::endl;
}

TEST(Run, Normal) {
    char arg0[] = "testrun";
    char arg1[] = "--gear";
    std::string str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/gear.json").string();
    std::cout << str << std::endl;
    std::vector<char> arg2(str.c_str(), str.c_str() + str.size() + 1);

    char arg3[] = "--rotation";
    str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/rotation.json").string();
    std::cout << str << std::endl;

    std::vector<char> arg4(str.c_str(), str.c_str() + str.size() + 1);

    char arg5[] = "--options";
    str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/options_non_varying.json").string();
    std::cout << str << std::endl;
    std::vector<char> arg6(str.c_str(), str.c_str() + str.size() + 1);

    char *argv[] = {&arg0[0], &arg1[0], arg2.data(), &arg3[0], arg4.data(), &arg5[0], arg6.data(), NULL};
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    ASSERT_NO_THROW(run(argc, argv));
}

TEST(Run, Varying) {
    char arg0[] = "testrun";
    char arg1[] = "--gear";
    std::string str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/gear.json").string();
    std::cout << str << std::endl;
    std::vector<char> arg2(str.c_str(), str.c_str() + str.size() + 1);

    char arg3[] = "--rotation";
    str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/rotation.json").string();
    std::cout << str << std::endl;

    std::vector<char> arg4(str.c_str(), str.c_str() + str.size() + 1);

    char arg5[] = "--options";
    str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/options_varying.json").string();
    std::cout << str << std::endl;
    std::vector<char> arg6(str.c_str(), str.c_str() + str.size() + 1);

    char *argv[] = {&arg0[0], &arg1[0], arg2.data(), &arg3[0], arg4.data(), &arg5[0], arg6.data(), NULL};
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    ASSERT_NO_THROW(run(argc, argv));
}

TEST(Run, NegativeDelay) {
    char arg0[] = "testrun";
    char arg1[] = "--gear";
    std::string str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/gear.json").string();
    std::cout << str << std::endl;
    std::vector<char> arg2(str.c_str(), str.c_str() + str.size() + 1);

    char arg3[] = "--rotation";
    str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/rotation.json").string();
    std::cout << str << std::endl;

    std::vector<char> arg4(str.c_str(), str.c_str() + str.size() + 1);

    char arg5[] = "--options";
    str = (std::filesystem::path(SIM_TEST_SOURCE_DIR) / "test/options_negative_delay.json").string();
    std::cout << str << std::endl;
    std::vector<char> arg6(str.c_str(), str.c_str() + str.size() + 1);

    char *argv[] = {&arg0[0], &arg1[0], arg2.data(), &arg3[0], arg4.data(), &arg5[0], arg6.data(), NULL};
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    ASSERT_NO_THROW(run(argc, argv));
}