#include "AbilityBuff.h"

namespace Simulator {
void AbilityBuff::apply(const Ability &aId, FinalStats &fstats, const Target &target) {}
void RawSheetBuff::apply(const Ability &aId, FinalStats &fstats, const Target &target) {
    if (_ids.empty() || std::find(_ids.begin(), _ids.end(), aId.id) != _ids.end()) {
        fstats.multiplier += _rawMultiplier;
        fstats.meleeRangeCritChance += _flatCritBonus;
        fstats.forceTechCritChance += _flatCritBonus;
        fstats.armorPen += _armorPen;
    }
}
void DamageTypeBuff::apply(const Ability &aId, FinalStats &fstats, const Target &target) {
    if (_types.empty() || std::find(_types.begin(), _types.end(), aId.damageType) != _types.end()) {
        fstats.multiplier += _rawMultiplier;
        fstats.meleeRangeCritChance += _flatCritBonus;
        fstats.forceTechCritChance += _flatCritBonus;
        fstats.armorPen += _armorPen;
    }
}
} // namespace Simulator
