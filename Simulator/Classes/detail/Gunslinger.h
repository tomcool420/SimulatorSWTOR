#pragma once
#include "Simulator/SimulatorBase/Ability.h"
#include "Simulator/SimulatorBase/ConditionalApplyDebuff.h"
#include "Simulator/SimulatorBase/DOT.h"
#include "Simulator/SimulatorBase/Target.h"
#include "Simulator/SimulatorBase/constants.h"

namespace Simulator::detail {
class GunslingerEntrencedOffenseBuff : public Buff {
  public:
    GunslingerEntrencedOffenseBuff() : Buff() {
        setId(gunslinger_entrenched_offsense);
        setDuration(Second(15));
    };
    [[nodiscard]] virtual DebuffEvents resolveEventsUpToTime(const Second &time, const TargetPtr &) {
        _stacks = std::clamp(static_cast<int>(std::floor((time - getStartTime()).getValue())), 0, 5);
        SIM_INFO("DEBUG: Time: {}, Entrenched Offense is now at {} stacks (start: {})", time, _stacks,
                 getStartTime().getValue());
        if (time > getEndTime()) {
            return {{DebuffEventType::Remove}};
        }
        return {};
    }
    virtual void apply(const Ability & /*ability*/, AllStatChanges &fstats, const TargetPtr & /*target*/) const {
        for (auto &&stats : fstats) {
            stats.multiplier += 0.03 * _stacks;
        }
    }
    [[nodiscard]] std::optional<Second> getNextEventTime() const final {
        if (_stacks < 5)
            return getStartTime() + ((_stacks + 1) * Second(1.0));
        return Buff::getNextEventTime();
    }
    [[nodiscard]] Buff *clone() const final { return new GunslingerEntrencedOffenseBuff(*this); };

  private:
    int _stacks{0};
};
inline AbilityPtr createDotAbility(AbilityInfo info, DOTPtr &&dot) {
    CHECK(dot);
    auto abl = std::make_shared<Ability>(dot->getId(), info);
    abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(std::move(dot)));
    return abl;
}

inline AbilityPtr createDotAbility(DOTPtr &&dot) {
    CHECK(dot);
    return createDotAbility(dot->getAbility().getInfo(), std::move(dot));
}

inline auto getTickOnWoundingShotsLambda() {
    return [](DamageHits &hits, const Second &time, const TargetPtr &source, const TargetPtr &target,
              DOT &dot) -> DamageHits {
        bool hitWoundingShots = false;
        for (auto &&hit : hits) {
            if (hit.id == dirty_fighting_wounding_shots && source->getId() == dot.getSource()->getId()) {
                hitWoundingShots = true;
                break;
            }
        }
        if (!hitWoundingShots)
            return {};
        return dot.tick(target, time, true);
    };
}

} // namespace Simulator::detail
