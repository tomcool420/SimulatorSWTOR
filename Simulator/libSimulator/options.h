#pragma once
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>

namespace Simulator {
struct Options {
    nlohmann::json gear;
    nlohmann::json rotation;
    int iterations{200};
    bool verbose{false};
    bool varying{true};
    int varyingStepSize{40};
    boost::filesystem::path output{"log.csv"};
};
void populateOptions(boost::program_options::options_description &desc);
Options parseOptions(const boost::program_options::variables_map &cmdLineArgs);
} // namespace Simulator
