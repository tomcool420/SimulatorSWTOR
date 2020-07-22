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
void RawSheetBuff::apply(const Ability &aId, AllStatChanges &fstats, const TargetPtr & /*target*/) const {
    if (_ids.empty() || std::find(_ids.begin(), _ids.end(), aId.getId()) != _ids.end()) {
        for (auto &&fstat : fstats) {
            fstat += _statChanges;
        }
    }
}
void DamageTypeBuff::apply(const Ability &abl, AllStatChanges &fstats, const TargetPtr & /*target*/) const {
    auto &&c = abl.getCoefficients();

    for (int ii = 0; ii < c.size(); ++ii) {
        if (_types.empty() || std::find(_types.begin(), _types.end(), c[ii].damageType) != _types.end()) {
            fstats[ii] += _statChanges;
        }
    }
}
DamageTypeBuff::DamageTypeBuff(const std::string &buffName, const std::vector<DamageType> &damageTypes,
                             double rawDamageMultipler, double flatCritBonus, double flatCritMultiplier, double ap)
    : _types(damageTypes), _name(buffName) {
    _statChanges.multiplier += rawDamageMultipler;
    _statChanges.flatMeleeRangeCritChance += flatCritBonus;
    _statChanges.flatMeleeRangeCriticalMultiplierBonus += flatCritMultiplier;
    _statChanges.flatForceTechCritChance += flatCritBonus;
    _statChanges.flatForceTechCriticalMultiplierBonus += flatCritMultiplier;
    _statChanges.armorPen += ap;
}
} // namespace Simulator
