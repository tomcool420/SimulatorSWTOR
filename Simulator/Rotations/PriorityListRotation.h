#pragma once
#include "PriorityList.h"
#include "StaticRotation.h"
#include <Simulator/SimulatorBase/Class.h>
#include <Simulator/SimulatorBase/Rotation.h>
#include <Simulator/SimulatorBase/types.h>

namespace Simulator {
class PriorityListRotation : public Rotation {
  public:
    PriorityListRotation(TargetPtr source) : Rotation(std::move(source)) {}
    void setClass(const ClassPtr &c) { _class = c; }
    void setPriorityList(const RotationalPriorityListPtr &plr) { _plr = plr; }
    void setOpener(const StaticRotationPtr &opener) { _opener = opener; }
    std::optional<Second> getNextEventTime() override;
    virtual AbilityPtr getNextAbility() override;
    virtual ~PriorityListRotation() = default;

  private:
    ClassPtr _class;
    RotationalPriorityListPtr _plr;
    std::optional<RotationalPriorityListPtr> _opener;
    // bool _openerDone{true};
    std::optional<std::pair<AbilityId, Second>> _nextAbility;
};
} // namespace Simulator