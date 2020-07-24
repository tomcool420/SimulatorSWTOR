#pragma once
#include "Ability.h"
#include "Debuff.h"
#include "utility.h"
#include <optional>

namespace Simulator {
class Target;
class DOT : public Debuff {
  public:
    DOT(AbilityId iid, double coeff, double ShxMin, double ShxMax, double Am, DamageType dt, bool dot, bool aoe,
        int ticks, Second tickRate, bool hasInitialTick)
        : DOT(iid, AbilityCoefficients{coeff, ShxMin, ShxMax, Am, dt, dot, aoe}, ticks, tickRate, hasInitialTick) {}

    DOT(AbilityId iid, AbilityCoefficients coeffs, int ticks, Second tickRate, bool hasInitialTick)
        : Debuff(iid), _DefaultTickRate(tickRate), _ability(iid, coeffs), _nticks(ticks), _defaultNTicks(ticks),
          _tickRate(tickRate), _hasInitialTick(hasInitialTick), _defaultHasInitialTick(hasInitialTick) {}
    DOT(AbilityId iid, AbilityInfo info) : Debuff(iid), _DefaultTickRate(info.time), _ability(iid, info) {
        replaceAbilityInfo(info);
    }
    [[nodiscard]] Debuff *clone() const override;

    void apply(const FinalStats &s, const Second &time) override {
        _tickRate = _DefaultTickRate / (1 + s.alacrity);
        return refresh(time);
    }
    DamageHits tick(const TargetPtr &t, const Second &time, bool extraTick = false);
    void refresh(const Second &time) override {
        if (_TickOnRefresh && _refreshCount > 0) {
            if (!_defaultHasInitialTick) {
                _hasInitialTick = true;
                _nticks = _defaultNTicks + 1;
            }
        }
        _TickCount = 0;
        _lastTickTime = Second(-1e6);
        setStartTime(time);
        setDuration((_nticks - _hasInitialTick) * _tickRate);
        ++_refreshCount;
    }
    [[nodiscard]] bool isFinished() const { return _TickCount == _nticks; }
    [[nodiscard]] const Ability &getAbility() const { return _ability; }
    [[nodiscard]] std::optional<Second> getNextEventTime() const override {
        if (_TickCount == _nticks)
            return std::nullopt;
        return std::max(_lastTickTime + _tickRate, getStartTime() + _tickRate * (!_hasInitialTick));
    }
    [[nodiscard]] DebuffEvents resolveEventsUpToTime(const Second &time, const TargetPtr &t) override;
    SIMULATOR_SET_MACRO(TickCount, int, 0);
    SIMULATOR_SET_MACRO(DoubleTickChance, std::optional<double>, std::nullopt);
    SIMULATOR_GET_ONLY_MACRO(DefaultTickRate, Second, Second{0.0});
    SIMULATOR_SET_MACRO(TickOnRefresh, bool, false);

    void replaceAbilityInfo(AbilityInfo info) {
        _nticks = info.nTicks;
        _defaultNTicks = info.nTicks;
        _tickRate = info.time;
        _hasInitialTick = info.extraInitialTick;
        _defaultHasInitialTick = info.extraInitialTick;
        _ability.setInfo(std::move(info));
    }

  private:
    Ability _ability;
    int _nticks;
    int _defaultNTicks;
    Second _tickRate;
    Second _lastTickTime{-1e6};
    bool _hasInitialTick{true};
    bool _defaultHasInitialTick{true};
    int _refreshCount{0};
};

void tickDot(DOT &dot, const TargetPtr &source, const TargetPtr &target, const Second &time);
using DOTPtr = std::unique_ptr<DOT>;

template <class T> class OnAbilityHitDot : public DOT {
  public:
    OnAbilityHitDot(AbilityId iid, AbilityCoefficients coeffs, int ticks, Second tickRate, bool hasInitialTick,
                    T &&call)
        : DOT(iid, coeffs, ticks, tickRate, hasInitialTick), _lambda(std::forward<T>(call)) {}
    OnAbilityHitDot(AbilityId iid, AbilityInfo info, T &&call) : DOT(iid, info), _lambda(std::forward<T>(call)) {}
    Debuff *clone() const override { return new OnAbilityHitDot<T>(*this); }
    [[nodiscard]] DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &source,
                                          const TargetPtr &target) override {
        return _lambda(hits, time, source, target, *this);
    }

  private:
    T _lambda;
};

template <class Lambda>
OnAbilityHitDot<Lambda> *MakeOnAbilityHitDot(AbilityId iid, AbilityCoefficients coeffs, int ticks, Second tickRate,
                                             bool hasInitialTick, Lambda &&call) {
    return new OnAbilityHitDot<Lambda>(iid, coeffs, ticks, tickRate, hasInitialTick, std::forward<Lambda>(call));
}
template <class Lambda> OnAbilityHitDot<Lambda> *MakeOnAbilityHitDot(AbilityId iid, AbilityInfo info, Lambda &&call) {
    return new OnAbilityHitDot<Lambda>(iid, info, std::forward<Lambda>(call));
}
} // namespace Simulator
