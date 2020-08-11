#include "AbilityDebuff.h"
#include "Ability.h"
#include "detail/log.h"

namespace Simulator {
void RawSheetDebuff::modifyStats(const Ability &ability, AllStatChanges &fstats,
                                 const std::shared_ptr<const Target> &) const {
    if (ability.getCoefficients().empty())
        return;
    auto &&c = ability.getCoefficients();
    CHECK(c.size() == fstats.size());
    if (_types.has_value()) {
        for (int ii = 0; ii < c.size(); ++ii) {
            if (_types->empty() || std::find(_types->begin(), _types->end(), c[ii].damageType) != _types->end()) {
                fstats[ii] += _statChanges;
            }
        }
    } else if (_ids.has_value()) {
        if (_ids->empty() || std::find(_ids->begin(), _ids->end(), ability.getId()) != _ids->end()) {
            for (int ii = 0; ii < c.size(); ++ii) {
                fstats[ii] += _statChanges;
            }
        }
    } else {
        for (int ii = 0; ii < c.size(); ++ii) {
            fstats[ii] += _statChanges;
        }
    }
}
void ShatteredDebuff::modifyStats(const Ability &ability, AllStatChanges &fstats,
                                  const std::shared_ptr<const Target> &) const {
    for (int ii = 0; ii < fstats.size(); ++ii) {
        auto &c = ability.getCoefficients()[ii];
        auto &s = fstats[ii];
        s.armorDebuff = true;
        s.multiplier += 0.05; // melee, range, force, tech
        if (c.damageType == DamageType::Internal || c.damageType == DamageType::Elemental) {
            s.multiplier += 0.07; // assailable;
        }
    }
}
DamageHits ShatteredDebuff::onAbilityHit(DamageHits &hits, const Second & /*time*/, const TargetPtr & /*player*/,
                                         const TargetPtr & /*target*/) {
    for (auto &&hit : hits) {
        if (hit.aoe) {
            hit.dmg *= 1.1;
        }
    }
    return {};
}
} // namespace Simulator
