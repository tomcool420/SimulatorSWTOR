#pragma once
#include "Buff.h"

namespace Simulator {
class ActiveStackingBuff : public Buff {
  public:
    void activate(const Second &time) override;
    [[nodiscard]] std::optional<Second> getNextEventTime() const override;
    [[nodiscard]] virtual DebuffEvents resolveEventsUpToTime(const Second &time, const TargetPtr &);

    SIMULATOR_SET_MACRO(ChargeCooldown, std::optional<Second>, Second(30));
    SIMULATOR_SET_MACRO(StacksOnActivation, int, 4);
    SIMULATOR_SET_MACRO(ChargeGainPerCooldown, int, 1);
    SIMULATOR_SET_MACRO(ConsumesChargesOnActivation, bool, true);
    SIMULATOR_SET_MACRO(CurrentCharges, int, 0);
    SIMULATOR_SET_MACRO(MaxCharges, int, 1);

  private:
    std::optional<Second> _nextChargeGain;
};
} // namespace Simulator