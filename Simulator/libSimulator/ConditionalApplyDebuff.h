#pragma once
#include "Debuff.h"
#include "OnHitAction.h"
#include "types.h"
#include "Target.h"

namespace Simulator {
class ConditionalApplyDebuff : public OnHitAction {
  public:
    explicit ConditionalApplyDebuff(DebuffPtr debuff, bool checkHit = true)
        : OnHitAction(), _debuff(std::move(debuff)), _checkHit(checkHit) {}
    void doOnHit(const DamageHits &hits, const TargetPtr &player, const TargetPtr &target, const Second &time) final {
        bool anyHit =
            std::find_if(hits.begin(), hits.end(), [](const DamageHit &hit) { return !hit.miss; }) != hits.end();
        if (anyHit || _checkHit) {
            target->addDebuff(std::unique_ptr<Debuff>(_debuff->clone()), player, time);
        }
    };

  private:
    DebuffPtr _debuff;
    bool _checkHit{true};
};

class ConditionalApplyBuff : public OnHitAction {
  public:
    explicit ConditionalApplyBuff(BuffPtr buff, bool checkHit = true)
        : OnHitAction(), _buff(std::move(buff)), _checkHit(checkHit) {}
    void doOnHit(const DamageHits &hits, const TargetPtr &player, const TargetPtr & /*target*/,
                 const Second &time) final {
        bool anyHit =
            std::find_if(hits.begin(), hits.end(), [](const DamageHit &hit) { return !hit.miss; }) != hits.end();
        if (anyHit || !_checkHit) {
            player->addBuff(std::unique_ptr<Buff>(_buff->clone()), time);
        }
    };

  private:
    BuffPtr _buff;
    bool _checkHit{true};
};
} // namespace Simulator
