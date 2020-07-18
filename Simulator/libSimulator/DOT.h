#pragma once
#include "Ability.h"
#include "Debuff.h"
#include <optional>

namespace Simulator {
class Target;
class DOT : public Debuff, public Ability {
  public:
    DOT(AbilityId iid, double coeff, double ShxMin, double ShxMax, double Am, DamageType dt, bool dot, bool aoe,
        int ticks, Second tickRate, bool hasInitialTick)
        : Debuff(), Ability(iid, coeff, ShxMin, ShxMax, Am, dt, dot, aoe), _nticks(ticks), _defaultTickRate(tickRate),
          _tickRate(tickRate),_hasInitialTick(hasInitialTick) {}

    DamageHits apply(const Target &t, const FinalStats &s, const Second &time) {
        _tickRate = _defaultTickRate / (1 + s.alacrity);
        return refresh(t, s, time);
    }
    DamageHits tick(const Target &t, const FinalStats &s, const Second &time);
    DamageHits refresh(const Target &t, const FinalStats &s, const Second &time) {
        _tickCount = 0;
        if (_hasInitialTick) {
            return tick(t, s, time);
        }
        _lastTickTime = time;
        return {};
    }
    [[nodiscard]] bool isFinished() const { return _tickCount == _nticks; }

    [[nodiscard]] std::optional<Second> getNextEvent() const {
        if (_tickCount == _nticks)
            return std::nullopt;
        return _lastTickTime + _tickRate;
    }
    void setPlayer(PlayerPtr player) {_player= player;}
    const PlayerPtr & getPlayer() const {return _player;}
    virtual ~DOT() = default;
  private:
    int _tickCount{0};
    int _nticks;
    Second _defaultTickRate;
    Second _tickRate;
    Second _lastTickTime;
    bool _hasInitialTick{true};
    PlayerPtr _player{nullptr};
};
using DOTPtr = std::unique_ptr<DOT>;
} // namespace Simulator
