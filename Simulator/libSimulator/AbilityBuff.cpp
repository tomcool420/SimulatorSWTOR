#include "AbilityBuff.h"

namespace Simulator {
RawSheetBuff::RawSheetBuff(const std::string &buffName, const AbilityIds &ids, double rawDamageMultipler,
                           double flatCritBonus, double flatCritMultiplier, double ap)
    : _ids(ids), _name(buffName) {
    _statChanges.multiplier += rawDamageMultipler;
    _statChanges.flatMeleeRangeCritChance += flatCritBonus;
    _statChanges.flatMeleeRangeCriticalMultiplierBonus += flatCritMultiplier;
    _statChanges.flatForceTechCritChance += flatCritBonus;
    _statChanges.flatForceTechCriticalMultiplierBonus += flatCritMultiplier;
    _statChanges.armorPen += ap;
}
void RawSheetBuff::apply(const Ability &aId, StatChanges &fstats, const TargetPtr & /*target*/) const {
    if (_ids.empty() || std::find(_ids.begin(), _ids.end(), aId.getId()) != _ids.end()) {
        fstats += _statChanges;
    }
}
void DamageTypeBuff::apply(const Ability &aId, StatChanges &fstats, const TargetPtr & /*target*/) const {
    if (_types.empty() || std::find(_types.begin(), _types.end(), aId.getCoefficients().damageType) != _types.end()) {
        fstats.multiplier += _rawMultiplier;
        fstats.flatMeleeRangeCritChance += _flatCritBonus;
        fstats.flatForceTechCritChance += _flatCritBonus;
        fstats.armorPen += _armorPen;
        fstats.flatForceTechCriticalMultiplierBonus += _flatCritMultiplier;
        fstats.flatMeleeRangeCriticalMultiplierBonus += _flatCritMultiplier;
    }
}
} // namespace Simulator
