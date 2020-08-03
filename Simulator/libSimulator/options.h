#pragma once
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>

namespace Simulator {
struct Options {
    nlohmann::json gear;
    nlohmann::json rotation;
    int iterations{1};
    bool verbose;
};
void populateOptions(boost::program_options::options_description &desc);
Options parseOptions(const boost::program_options::variables_map &cmdLineArgs);
} // namespace Simulator
