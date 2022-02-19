#include "calculations.h"
#include <cmath>

namespace Simulator::detail {

double getCriticalChance(Mastery mastery) {
    return 0.2 * (1 - std::pow(1 - 0.01 / 0.2, mastery.getValue() / (80 * 12.93)));
}
double getCriticalChance(CriticalRating crit) {
    return 0.3 * (1 - std::pow(1 - 0.01 / 0.3, crit.getValue() / (80 * 2.41)));
}
double getCriticalMultiplier(CriticalRating crit) {
    return 0.3 * (1 - std::pow(1 - 0.01 / 0.3, crit.getValue() / (80 * 2.41)));
}
double getAlacrity(AlacrityRating alacrity) {
    return 0.3 * (1 - std::pow(1 - 0.01 / 0.3, alacrity.getValue() / (80* 3.2)));
}
double getFTBonusDamage(FTPower ftbonus) { return ftbonus.getValue() * 0.23; }
double getBonusDamage(Mastery mastery) { return mastery.getValue() * 0.2000; }
double getBonusDamage(Power power) { return power.getValue() * 0.23; }
double getDamageReduction(Armor armor) {
    return (armor.getValue() / (armor.getValue() + 390.0 * 80 + 800));
}
Second getReducedDuration(Second time, double alacrity) { return time / (1.0 + alacrity); }

} // namespace Simulator::detail
