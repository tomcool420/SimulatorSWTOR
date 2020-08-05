#include "run.h"
#include "factory.h"
#include <Simulator/Rotations/PriorityListRotation.h>
#include <Simulator/SimulatorBase/AbilityBuff.h>
#include <Simulator/SimulatorBase/Target.h>
namespace Simulator {
namespace po = boost::program_options;
int run(int argc, char **argv) {
    boost::program_options::options_description desc("Options");
    populateOptions(desc);
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);
    } catch (const po::error &e) {
        SIM_INFO("Error: {}", e.what());
    }
    Options o = parseOptions(vm);
    auto ic = getClassFromGearJSON(o.gear);
    auto rs = getRawStatsFromGearJSON(o.gear);

    auto s = Target::New(rs);
    auto amps = getAmplifiersFromGearJSON(o.gear);
    auto buff = std::make_unique<AmplifierBuff>(amps);
    s->addBuff(std::move(buff), Second(0.0));

    RawStats trs;
    trs.hp = HealthPoints(6500000);
    auto t = Target::New(trs);

    auto rotation = RotationalPriorityList::deserialize(o.rotation);

    PriorityListRotation rot(s);
    rot.setTarget(t);
    rot.setClass(ic);
    rot.setPriorityList(rotation);
    rot.doRotation();
    return 0;
}

} // namespace Simulator