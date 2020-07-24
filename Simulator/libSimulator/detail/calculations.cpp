#include "calculations.h"
#include <cmath>

namespace Simulator::detail {
[[nodiscard]] double getCriticalChance(Mastery mastery) {
    return 0.2 * (1 - std::pow(1 - 0.01 / 0.2, mastery.getValue() / (75 * 8.43)));
}
[[nodiscard]] double getCriticalChance(CriticalRating crit) {
    return 0.3 * (1 - std::pow(1 - 0.01 / 0.3, crit.getValue() / (75 * 1.503)));
}
[[nodiscard]] double getCriticalMultiplier(CriticalRating crit){
    return 0.3 * (1 - std::pow(1 - 0.01 / 0.3, crit.getValue() / (75 * 1.503)));
}
[[nodiscard]] double getAlacrity(AlacrityRating alacrity) {
    return 0.3 * (1 - std::pow(1 - 0.01 / 0.3, alacrity.getValue() / (75 * 2.016)));
}
[[nodiscard]] double getFTBonusDamage(FTPower ftbonus){
    return ftbonus.getValue()*0.23;
}
[[nodiscard]] double getBonusDamage(Mastery mastery){
    return mastery.getValue()*0.20001;
}
[[nodiscard]] double getBonusDamage(Power power){
    return power.getValue()*0.23;
}
[[nodiscard]] double getDamageReduction(Armor armor){
    return (armor.getValue()/(armor.getValue()+349.0*75+800));
}
[[nodiscard]] Second getReducedDuration(Second time, double alacrity){
    return time /(1.0+alacrity);
}

} // namespace Simulator::detail
