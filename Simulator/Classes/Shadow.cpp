#include "Shadow.h"
#include "Simulator/SimulatorBase/abilities.h"
#include "detail/Consular.h"
#include "detail/shared.h"
#include <Simulator/SimulatorBase/constants.h>

namespace Simulator {
namespace detail {
class ForceSynergy : public Buff {
  public:
    ForceSynergy() {
        setMaxStacks(1);
        setStackDuration(Second(10));
    }

    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr &target) const override {
        if (getCurrentStacks() == 0)
            return;
        for (int ii = 0; ii < fstats.size(); ++ii) {
            auto &&c = ability.getCoefficients()[ii];
            auto &fs = fstats[ii];
            if (c.damageType == DamageType::Weapon)
                fs.flatMeleeRangeCritChance = 0.05;
        }
    }
    DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr & /*player*/,
                            const TargetPtr & /*target*/) override {
        for (auto &&hit : hits) {
            if (!hit.weapon)
                activate(time);
        }
        return {};
    }
};

class AppliedForce : public Buff {
    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr &target) const override {
        auto id = ability.getId();
        if (id == consular_double_strike || id == infiltration_clairvoyant_strike) {
            for (int ii = 0; ii < fstats.size(); ++ii) {
                auto &fs = fstats[ii];
                fs.multiplier += 0.05;
            }
        }
    }
};
} // namespace detail
std::vector<BuffPtr> Shadow::getStaticBuffs() {
    auto buffs = ClassBase::getStaticBuffs();
    buffs.push_back(std::make_unique<detail::ForcePotency>(2 + getForceHarmonics()));
    buffs.push_back(std::make_unique<detail::ForceSynergy>());
    buffs.push_back(std::make_unique<detail::AppliedForce>());
    return buffs;
}

AbilityPtr Shadow::getAbilityInternal(AbilityId id) {
    switch (id) {
    case consular_saber_strike:
        return std::make_shared<Ability>(id, detail::getDefaultAbilityInfo(id));
    case consular_double_strike:
        return std::make_shared<Ability>(id, detail::getDefaultAbilityInfo(id));
    case shadow_shadow_strike:
        return std::make_shared<Ability>(id, detail::getDefaultAbilityInfo(id));
    case shadow_spinning_strike:
        return std::make_shared<Ability>(id, detail::getDefaultAbilityInfo(id));
    }
    return nullptr;
}
} // namespace Simulator