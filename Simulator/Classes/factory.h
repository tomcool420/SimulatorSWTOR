#pragma once
#include <Simulator/SimulatorBase/types.h>
namespace Simulator {
ClassPtr createClassFromJson(const nlohmann::json &j);
}