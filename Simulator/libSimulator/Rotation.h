#pragma once
#include "types.h"
#include "utility.h"
#include <optional>
#include "types.h"
#include "utility.h"
namespace Simulator {
class Rotation {
  public:
    Rotation(TargetPtr source) : _source(source) {}
    
    virtual std::optional<Second> getNextEventTime()=0;
    virtual void resolveEventsUpToTime(const Second &time, const TargetPtr &)=0;

    const TargetPtr &getSource() const {return _source;}
    virtual ~Rotation()=default;
  private:
    TargetPtr _source;
};

class SetRotation : public Rotation{
  public:
    SetRotation(TargetPtr source,std::vector<AbilityId> ids) : Rotation(source),_ids(std::move(ids)) {}
    std::optional<Second> getNextEventTime() override;
    void setStart(Second time){_nextFreeGCD=time;}
    void resolveEventsUpToTime(const Second &time, const TargetPtr &) override;
    void doRotation();
    void setTarget(TargetPtr &target){_target=target;}
    virtual ~SetRotation() = default;
    
    SIMULATOR_SET_MACRO(DelayAfterChanneled,Second,Second{0.0})
    SIMULATOR_SET_MACRO(Repeats,int,1);
  private:
    AbilityIds _ids;
    AbilityPtr _currentAbility{nullptr};
    int _idCounter{0};
    int _iterationCount{0};
    Second _nextFreeGCD{-1000.0};
    std::optional<Second> _lastEvent;
    int _currentTick{0};

    double currentAbilityPercent{0.0};
    TargetPtr _target{nullptr};
    


};
} // namespace Simulator
