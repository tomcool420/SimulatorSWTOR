#include "run.h"
#include "factory.h"
#include <Simulator/Classes/detail/shared.h>
#include <Simulator/Rotations/PriorityListRotation.h>
#include <Simulator/SimulatorBase/AbilityBuff.h>
#include <Simulator/SimulatorBase/Target.h>
#include <Simulator/SimulatorBase/parseHelpers.h>
#include <fstream>
#include <tbb/parallel_for.h>

namespace Simulator {
namespace po = boost::program_options;
constexpr char kHelpFlag[] = "help";

int run(int argc, char **argv) {
    boost::program_options::options_description desc("Options");
    populateOptions(desc);
    desc.add_options()(kHelpFlag, "Print out usage imformation");

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        if (vm.count(kHelpFlag)) {
            SIM_INFO("{}", desc);
            exit(0);
        }
        po::notify(vm);
    } catch (const po::error &e) {
        SIM_INFO("Error: {}", e.what());
        SIM_INFO("{}", desc);
        exit(1);
    }
    try {
        Options o = parseOptions(vm);
        if (o.opts.varyingAlacrity) {
            runVarying(o);
        } else {
            runRotation(o);
        }
    } catch (const std::exception &e) {
        SIM_INFO("Unexpected exception : {}", e.what());
        exit(1);
    }
    return 0;
}
namespace {
struct stats {
    Second mean;
    double stddev;
    Second min;
    Second max;
};
stats getStdDev(const std::vector<Second> &times) {
    Second max{0.0};
    Second min{1e6};
    for (auto &t : times) {
        min = std::min(t, min);
        max = std::max(t, max);
    }
    auto totalTime = std::accumulate(times.begin(), times.end(), Second(0.0));
    auto mean = totalTime / static_cast<double>(times.size());
    std::vector<double> diffsSq(times.size());
    std::transform(times.begin(), times.end(), diffsSq.begin(), [=](const Second &a) -> double {
        auto v = (a - mean).getValue();
        return v * v;
    });
    auto variance = std::accumulate(diffsSq.begin(), diffsSq.end(), 0.0) / times.size();
    auto stddev = std::sqrt(variance);
    return {mean, stddev, min, max};
}
} // namespace
int runRotation(const Options &o) {
    auto ic = getClassFromGearJSON(o.gear);
    auto rs = getRawStatsFromGearJSON(o.gear);
    auto &&ro = o.opts;
    for (int ii = 0; ii < ro.iterations; ++ii) {
        detail::LogDisabler *d{nullptr};
        if (!ro.verbose)
            d = new detail::LogDisabler;
        auto s = Target::New(rs);
        auto amps = getAmplifiersFromGearJSON(o.gear);
        auto buff = std::make_unique<AmplifierBuff>(amps);
        s->addBuff(std::move(buff), Second(0.0));
        s->addBuff(detail::getDefaultStatsBuffPtr(false, false), Second(0.0));
        addBuffs(s, ic->getStaticBuffs(), Second(0.0));
        RawStats trs;
        trs.hp = HealthPoints(6500000);
        auto t = Target::New(trs);
        t->addDebuff(detail::getGenericDebuff(debuff_shattered), s, Second(0.0));
        auto rotationJSON = o.rotation.at(key_main_rotation);
        std::cout << rotationJSON << std::endl;
        auto rotation = RotationalPriorityList::deserialize(rotationJSON);

        PriorityListRotation rot(s);
        rot.setNextFreeGCD(Second(0.0));
        rot.setTarget(t);
        rot.setClass(ic);
        rot.setPriorityList(rotation);
        rot.setDelayAfterChanneled(ro.delayAfterChanneling);
        rot.setMinTimeAfterInstant(ro.delayAfterInstant);
        rot.doRotation();
        auto &&events = t->getEvents();
        if (d) {
            delete d;
            d = nullptr;
        }
        logParseInformation(events, getLastDamageEvent(events) - getFirstDamageEvent(events));
    }

    return 0;
}
int runVarying(const Options &opts) {
    auto runLambda = [](const Options &o, AlacrityRating ala, CriticalRating crit) -> TargetPtr {
        auto ic = getClassFromGearJSON(o.gear);
        auto rs = getRawStatsFromGearJSON(o.gear);
        rs.alacrityRating = ala;
        rs.criticalRating = crit;
        auto s = Target::New(rs);
        auto amps = getAmplifiersFromGearJSON(o.gear);
        auto buff = std::make_unique<AmplifierBuff>(amps);
        s->addBuff(std::move(buff), Second(0.0));
        addBuffs(s, ic->getStaticBuffs(), Second(0.0));
        RawStats trs;
        trs.hp = HealthPoints(6500000);
        auto t = Target::New(trs);
        t->addDebuff(detail::getGenericDebuff(debuff_shattered), s, Second(0.0));
        auto rotation = RotationalPriorityList::deserialize(o.rotation.at(key_main_rotation));

        PriorityListRotation rot(s);
        rot.setNextFreeGCD(Second(0.0));
        rot.setTarget(t);
        rot.setClass(ic);
        rot.setDelayAfterChanneled(o.opts.delayAfterChanneling);
        rot.setMinTimeAfterInstant(o.opts.delayAfterInstant);
        rot.setPriorityList(rotation);
        rot.doRotation();
        return std::move(t);
    };
    struct info {
        AlacrityRating alacrity;
        CriticalRating crit;
        double mean;
        double stddev;
    };
    auto rs = getRawStatsFromGearJSON(opts.gear);
    int iterations = opts.opts.iterations;
    int stepSize = opts.opts.varyingStepSize;
    double totalVaryingStats = rs.criticalRating.getValue() + rs.alacrityRating.getValue();
    int count = static_cast<int>(totalVaryingStats / stepSize);
    std::vector<info> infos(count);
    auto ld = new detail::LogDisabler;
    tbb::parallel_for(tbb::blocked_range<int>(0, count), [&](const tbb::blocked_range<int> &r) {
        for (int ii = r.begin(); ii < r.end(); ++ii) {
            double alacrity = ii * stepSize;
            std::cout << "Alacrity: " << alacrity << "\n";
            double crit = std::max(0.0, totalVaryingStats - alacrity);
            std::vector<Second> times;
            for (int iit = 0; iit < iterations; ++iit) {
                auto &&t = runLambda(opts, AlacrityRating(alacrity), CriticalRating(crit));
                auto &&events = t->getEvents();
                auto time = getLastDamageEvent(events) - getFirstDamageEvent(events);
                times.push_back(time);
            }
            auto &&[mean, stddev, minV, maxV] = getStdDev(times);
            infos[ii] = info{AlacrityRating(alacrity), CriticalRating(crit), mean.getValue(), stddev};
        }
    });
    delete ld;
    std::ofstream f(opts.output.string());
    f << "alacrity rating,critical rating,mean,stddev"
      << "\n";

    for (int ii = 0; ii < infos.size(); ++ii) {
        auto &&vc = infos[ii];
        f << vc.alacrity.getValue() << "," << vc.crit.getValue() << "," << vc.mean << "," << vc.stddev << "\n";
    }
    return 0;
}

} // namespace Simulator