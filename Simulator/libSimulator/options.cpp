#include "options.h"
#include <Simulator/SimulatorBase/utility.h>
#include <fstream>
#include <string>
namespace Simulator {
namespace po = boost::program_options;
namespace fs = boost::filesystem;
constexpr char kInputGearFlag[] = "gear";
constexpr char kInputRotationFlag[] = "rotation";
constexpr char kIterationFlag[] = "iterations";
#define OPTION(__fn, __sn) (std::string(__fn) + ","__sn).c_str();

void populateOptions(po::options_description &desc) {
    desc.add_options()(kInputGearFlag, po::value<std::string>()->required(),
                       "(Required) Gear json file containing raw stats and gear information");
    desc.add_options()(kInputRotationFlag, po::value<std::string>()->required(), "(Required) Rotation json file");
    desc.add_options()(kIterationFlag, po::value<int>()->default_value(1),
                       "(Optional) variable defining how many times the rotation should run");
}

Options parseOptions(const po::variables_map &cmdLineArgs) {
    Options options;

    {
        fs::path p = cmdLineArgs[kInputGearFlag].as<std::string>();
        CHECK(fs::exists(p));
        std::ifstream f(p.string());
        options.gear = nlohmann::json::parse(f);
    }
    {
        fs::path p = cmdLineArgs[kInputRotationFlag].as<std::string>();
        CHECK(fs::exists(p));
        std::ifstream f(p.string());
        options.rotation = nlohmann::json::parse(f);
    }
    if (cmdLineArgs.count(kIterationFlag)) {
        options.iterations = cmdLineArgs[kIterationFlag].as<int>();
    }
    return options;
}

} // namespace Simulator