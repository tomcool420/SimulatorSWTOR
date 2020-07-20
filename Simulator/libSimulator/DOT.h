#pragma once
#include "Ability.h"
#include "Debuff.h"
#include <optional>

namespace Simulator {
class Target;
class DOT : public Debuff {
  public:
    DOT(AbilityId iid, double coeff, double ShxMin, double ShxMax, double Am, DamageType dt, bool dot, bool aoe,
        int ticks, Second tickRate, bool hasInitialTick)
        : Debuff(), _ability(iid, coeff, ShxMin, ShxMax, Am, dt, dot, aoe), _nticks(ticks), _defaultTickRate(tickRate),
          _tickRate(tickRate), _hasInitialTick(hasInitialTick) {}
    [[nodiscard]] Debuff *clone() const override;
    void apply( Target &t, const FinalStats &s, const Second &time) {
        _tickRate = _defaultTickRate / (1 + s.alacrity);
        return refresh(t, time);
    }
    DamageHits tick(Target &t, const Second &time);
    void refresh(Target &t, const Second &time) {
        _tickCount = 0;
        _lastTickTime=Second(-1e6);
        setStartTime(time);
        setEndTime(time+(_nticks-_hasInitialTick)*_tickRate);
    }
    [[nodiscard]] bool isFinished() const { return _tickCount == _nticks; }
    [[nodiscard]] const AbilityId &getId() const { return _ability.getId(); }
    [[nodiscard]] const Ability &getAbility() const { return _ability; }
    [[nodiscard]] std::optional<Second> getNextEventTime() const override{
        if (_tickCount == _nticks)
            return std::nullopt;
        return std::max(_lastTickTime + _tickRate,getStartTime()+_tickRate*(!_hasInitialTick));
    }
    [[nodiscard]] DebuffEvents resolveEventsUpToTime(const Second & time, Target &t) override;

  private:
    Ability _ability;
    int _tickCount{0};
    int _nticks;
    Second _defaultTickRate;
    Second _tickRate;
    Second _lastTickTime{-1e6};
    std::optional<double> _doubleTickChance;
    bool _hasInitialTick{true};
};

void tickDot(DOT & dot, Target &source, Target & target, const Second &time);
using DOTPtr = std::unique_ptr<DOT>;
} // namespace Simulator
