#pragma once
#include "Ability.h"
#include <nlohmann/json.hpp>
namespace Simulator::detail {
AbilityInfo getAbilityFromJson(const nlohmann::json & json);
AbilityInfo getDefaultAbilityInfo(AbilityId id);
} // namespace Simulator::detail
