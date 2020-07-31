#pragma once
#include "../../libSimulator/types.h"

namespace Simulator::detail {
DebuffPtr getGenericDebuff(AbilityId id);
BuffPtr getGenericBuff(AbilityId id);
BuffPtr getDefaultStatsBuffPtr(bool twopiece, bool forceValor);
} // namespace Simulator::detail
