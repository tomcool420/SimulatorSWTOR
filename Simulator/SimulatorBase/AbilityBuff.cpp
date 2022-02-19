#include "AbilityBuff.h"
#include "detail/names.h"

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
DamageHits RelicProcBuff::onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &player,
                                       const TargetPtr & /*target*/) {
    bool procced = false;
    if (!_nextAvailableProc || (*_nextAvailableProc) < time) {
        // we have a chance to proc here
        for (int ii = 0; ii < hits.size(); ++ii) {
            if (getRandomValue(0.0, 1.0) >= 0.7) {
                procced = true;
                break;
            }
        }
    }
    if (procced) {
        StatChanges sc;
        sc.masteryBonus = _mastery;
        sc.powerBonus = _power;
        sc.criticalRatingBonus = _criticalRating;
        auto buff = std::make_unique<RawSheetBuff>("Surge", AbilityIds{}, sc);
        buff->setStartTime(time);
        buff->setDuration(getBuffDuration());
        buff->setId(_procId);
        player->addBuff(std::move(buff), time);
        _nextAvailableProc = time + getBuffCooldown();
    }
    return {};
}


} // namespace Simulator
