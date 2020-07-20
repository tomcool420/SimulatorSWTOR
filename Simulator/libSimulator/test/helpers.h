#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../StatBuff.h"
#include "../detail/calculations.h"

namespace Simulator {
AbilityPtr getAbility(AbilityId id);
std::vector<BuffPtr> getTacticsSheetBuffs();
StatChanges getDefaultStatsBuffs(bool twoPiece = true, bool mastery = true);
RawStats getDefaultStats();
BuffPtr getDefaultStatsBuffPtr(bool twoPiece = true, bool mastery = true);
} // namespace Simulator
