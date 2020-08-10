#pragma once
#include <Simulator/SimulatorBase/detail/units.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>

namespace Simulator {
struct RotationOptions {
    int iterations{200};
    bool verbose{false};
    bool varyingAlacrity{false};
    int varyingStepSize{40};
    Second delayAfterChanneling{0.0};
    Second delayAfterInstant{0.0};
};

struct Options {
    nlohmann::json gear;
    nlohmann::json rotation;
    RotationOptions opts;
    boost::filesystem::path output{"log.csv"};
};

void populateOptions(boost::program_options::options_description &desc);
Options parseOptions(const boost::program_options::variables_map &cmdLineArgs);
} // namespace Simulator
