#pragma once
#include <Simulator/SimulatorBase/ActiveStackingBuff.h>

namespace Simulator::detail {
class ForceClarityBuff : public ActiveStackingBuff {
  public:
    ForceClarityBuff() {
        setCurrentCharges(4);
        setMaxStacks(4);
        setMaxCharges(4);
        setStacksOnActivation(getMaxStacks());
        setId(force_clarity);
        setChargeCooldown(Second(30));
    }
    void activate(const Second &time);
    [[nodiscard]] std::optional<Second> getNextEventTime() const override;
    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr &target) const override;
};
} // namespace Simulator::detail