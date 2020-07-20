#pragma once
#include "Debuff.h"
#include "OnHitAction.h"
#include "types.h"

namespace Simulator {
class ConditionalApplyDebuff : public OnHitAction {
public:
    explicit ConditionalApplyDebuff(DebuffPtr debuff) : OnHitAction(),  _debuff(std::move(debuff)){}
    void doOnHit(const DamageHits &hits, const TargetPtr &player,const TargetPtr &target, const Second &time) final {
        bool anyHit =
            std::find_if(hits.begin(), hits.end(), [](const DamageHit &hit) { return !hit.miss; }) != hits.end();
        if (anyHit) {
            target->addDebuff(std::unique_ptr<Debuff>(_debuff->clone()), player, time);
        }
    };

  private:
    DebuffPtr _debuff;
};
} // namespace Simulator
