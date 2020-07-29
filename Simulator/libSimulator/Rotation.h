#pragma once
#include "types.h"
#include "utility.h"
#include <optional>
namespace Simulator {
class Rotation {
  public:
    Rotation(TargetPtr source) : _source(source) {}

    virtual std::optional<Second> getNextEventTime();
    virtual void resolveEventsUpToTime(const Second &time, const TargetPtr &);

    const TargetPtr &getSource() const { return _source; }
    void setTarget(TargetPtr &target) { _target = target; }
    const TargetPtr &getTarget() const { return _target; }
    [[nodiscard]] Second getNextFreeGCD() const { return _nextFreeGCD; }
    [[nodiscard]] Second getNextFreeGCDForInstantCast() const { return _nextFreeGCDForInstant; }
    virtual AbilityPtr getNextAbility() = 0;
    void doRotation();

    virtual ~Rotation() = default;

    void setNextFreeGCD(Second time) {
        _nextFreeGCD = time;
        _nextFreeGCDForInstant = time;
    }
    SIMULATOR_SET_MACRO(DelayAfterChanneled, Second, Second{0.0})
    SIMULATOR_SET_MACRO(MinTimeAfterInstant, Second, Second{0.1})

  private:
    TargetPtr _source;
    TargetPtr _target{nullptr};
    AbilityPtr _currentAbility{nullptr};
    Second _nextFreeGCD{-1000.0};
    Second _nextFreeGCDForInstant{-1000.0};
    std::optional<Second> _lastEvent;
    int _currentTick{0};
    double _abilityAlacrityAmount{0.0};
    Second _abilityCastTickTime{0.0};
    Second _abilityStartTime{-1000.0};
};

class SetRotation : public Rotation {
  public:
    SetRotation(TargetPtr source, std::vector<AbilityId> ids) : Rotation(source), _ids(std::move(ids)) {}
    std::optional<Second> getNextEventTime() override;
    void setStart(Second time) { setNextFreeGCD(time); }
    virtual ~SetRotation() = default;
    void setClass(const ClassPtr &c) { _class = c; }
    SIMULATOR_SET_MACRO(Repeats, int, 1);
    AbilityPtr getNextAbility() override;

  private:
    AbilityIds _ids;
    ClassPtr _class;
    int _idCounter{0};
    int _iterationCount{0};
};
} // namespace Simulator
