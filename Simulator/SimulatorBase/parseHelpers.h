#pragma once
#include "Target.h"
#include "types.h"
namespace Simulator {
void logParseInformation(const Target::TargetEvents &events, Second duration);
Second getFirstDamageEvent(const Target::TargetEvents &events);
Second getLastDamageEvent(const Target::TargetEvents &events);
} // namespace Simulator