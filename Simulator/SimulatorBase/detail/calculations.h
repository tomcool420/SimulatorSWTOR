#pragma once
#include "units.h"
namespace Simulator::detail {
[[nodiscard]] double getCriticalChance(Mastery mastery);
[[nodiscard]] double getCriticalChance(CriticalRating crit);
[[nodiscard]] double getCriticalMultiplier(Mastery mastery);
[[nodiscard]] double getCriticalMultiplier(CriticalRating crit);
[[nodiscard]] double getAlacrity(AlacrityRating alacrity);
[[nodiscard]] double getFTBonusDamage(FTPower ftbonus);
[[nodiscard]] double getBonusDamage(Mastery mastery);
[[nodiscard]] double getBonusDamage(Power power);
[[nodiscard]] double getDamageReduction(Armor armor);
[[nodiscard]] Second getReducedDuration(Second time, double alacrity);
} // namespace Simulator::detail
