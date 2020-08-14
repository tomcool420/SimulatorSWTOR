#pragma once
#include <Simulator/SimulatorBase/ActiveStackingBuff.h>
namespace Simulator::detail {
class ForcePotency : public ActiveStackingBuff {
  public:
    ForcePotency(int stacks = 2) {
        setChargeCooldown(Second(90));
        setStackDuration(Second(20));
        setStacksOnActivation(stacks);
        setChargeGainPerCooldown(stacks);
        setMaxStacks(stacks);
        setCurrentCharges(stacks);
    }

    [[nodiscard]] DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &player,
                                          const TargetPtr &target) override {

        auto sc = getCurrentStacks();
        if (!sc)
            return {};
        for (auto &&hit : hits) {
            if (sc > 0 && hit.direct && !(hit.weapon) && !(hit.miss))
                --sc;
        }
        setCurrentStacks(sc, time);
        return {};
    }

    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr &target) const override {
        if (!getCurrentStacks())
            return;
        for (int ii = 0; ii < fstats.size(); ++ii) {
            auto &&c = ability.getCoefficients()[ii];
            auto &fs = fstats[ii];
            if (c.isDirectDamage && !(c.damageType == DamageType::Weapon))
                fs.flatForceTechCritChance = 0.6;
        }
    }
};
} // namespace Simulator::detail