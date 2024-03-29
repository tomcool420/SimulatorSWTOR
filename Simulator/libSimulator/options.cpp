#include "options.h"
#include <Simulator/SimulatorBase/utility.h>
#include <fstream>
#include <string>
namespace Simulator {
namespace po = boost::program_options;
namespace fs = std::filesystem;
constexpr char kInputGearFlag[] = "gear";
constexpr char kInputRotationFlag[] = "rotation";
constexpr char kInputRotationOptionsFlag[] = "options";
#define OPTION(__fn, __sn) (std::string(__fn) + ","__sn).c_str();
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RotationOptions, iterations, verbose, varyingAlacrity, varyingStepSize,
                                   delayAfterChanneling, delayAfterInstant);

void populateOptions(po::options_description &desc) {
    desc.add_options()(kInputGearFlag, po::value<std::string>()->required(),
                       "(Required) Gear json file containing raw stats and gear information");
    desc.add_options()(kInputRotationFlag, po::value<std::string>()->required(), "(Required) Rotation json file");
    desc.add_options()(kInputRotationOptionsFlag, po::value<std::string>(),
                       "(Optional) File describing the expected run");
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
    if (cmdLineArgs.count(kInputRotationOptionsFlag)) {
        fs::path p = cmdLineArgs[kInputRotationOptionsFlag].as<std::string>();
        CHECK(fs::exists(p));
        std::ifstream f(p.string());
        options.opts = nlohmann::json::parse(f);
    } else {
        nlohmann::json j = options.opts;
        std::cout << j << std::endl;
    }
    return options;
}

} // namespace Simulator
