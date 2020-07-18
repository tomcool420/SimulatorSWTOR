#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../StatBuff.h"
#include "../detail/calculations.h"

namespace Simulator {
std::vector<BuffPtr> getTacticsSheetBuffs();
StatChanges getDefaultStatsBuffs(bool twoPiece = true);
RawStats getDefaultStats();
BuffPtr getDefaultStatsBuffPtr();
}
