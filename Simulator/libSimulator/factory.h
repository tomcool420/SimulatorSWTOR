#pragma once
#include <Simulator/SimulatorBase/types.h>

namespace Simulator {
ClassPtr getClassFromGearJSON(const nlohmann::json &j);
RawStats getRawStatsFromGearJSON(const nlohmann::json &j);
Amplifiers getAmplifiersFromGearJSON(const nlohmann::json &j);
} // namespace Simulator