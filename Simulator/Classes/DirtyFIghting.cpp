#include "DirtyFighting.h"
#include "detail/Gunslinger.h"
#include "detail/shared.h"
#include "../libSimulator/AbilityBuff.h"
#include "../libSimulator/ConditionalApplyDebuff.h"
#include "../libSimulator/abilities.h"

namespace Simulator {
namespace detail {
class BloodyMayhem : public Debuff {
  public:
    BloodyMayhem() : Debuff(dirty_fighting_bloody_mayhem) { setDuration(Second(15.0)); }

    DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &player,
                            const TargetPtr &target) override {
        if (_triggered)
            return {};
        AbilityCoefficients coeffs{0.396, 0.0396, 0.0396, 0.0, DamageType::Internal};
        AbilityInfo info{{coeffs}};
        Ability abl(dirty_fighting_bloody_mayhem, info);
        for (auto &&hit : hits) {
            if (hit.id == dirty_fighting_shrap_bomb) {

                _triggered = true;
                break;
            }
        }
        if (_triggered) {
            DamageHits ret;
            auto afs = getAllFinalStats(abl, player, target);
            auto newHits = getHits(abl, afs, target);
            ret.insert(ret.end(), newHits.begin(), newHits.end());
            setDuration(time - getStartTime() + Second(1e-5));
            return ret;
        }
        return {};
    }
    [[nodiscard]] Debuff *clone() const override { return new BloodyMayhem(*this); }

  private:
    bool _triggered{false};
};

class ColdBlooded : public Buff {
  public:
    ColdBlooded() : Buff() {}

    DamageHits onAbilityHit(DamageHits &hits, const Second & /*time*/, const TargetPtr & /*player*/,
                            const TargetPtr &target) override {
        if (target->getCurrentHealth() / target->getMaxHealth() < 0.3) {
            for (auto &&hit : hits) {
                if (hit.id == dirty_fighting_exploited_weakness || hit.id == dirty_fighting_shrap_bomb ||
                    hit.id == gunslinger_vital_shot ||
                    (hit.id == dirty_fighting_dirty_blast && hit.dt == DamageType::Internal)) {
                    hit.dmg *= 1.15;
                }
            }
        }
        return {};
    }
    [[nodiscard]] Buff *clone() const override { return new ColdBlooded(*this); }
};
} // namespace detail

AbilityPtr DirtyFighting::getAbilityInternal(AbilityId id) {
    switch (id) {
    case dirty_fighting_dirty_blast: {
        auto info = detail::getDefaultAbilityInfo(id);
        auto abl = std::make_shared<Ability>(dirty_fighting_dirty_blast, std::move(info));
        StatChanges sc;
        sc.multiplier = 0.1;
        abl->setStatChanges(sc);
        if (getExploitedWeakness()) {
            auto ewInfo = detail::getDefaultAbilityInfo(dirty_fighting_exploited_weakness);
            abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(
                std::make_unique<DOT>(dirty_fighting_exploited_weakness, ewInfo)));
        }
        return abl;
    }
    case dirty_fighting_wounding_shots: {
        auto info = detail::getDefaultAbilityInfo(dirty_fighting_wounding_shots);
        auto ws = std::make_shared<Ability>(id, std::move(info));
        ws->addOnEndAction([](const TargetPtr &source, const TargetPtr &, const Second &time) {
            if (auto b = source->getBuff<Buff>(gunslinger_smugglers_luck)) {
                source->removeBuff(gunslinger_smugglers_luck, time);
            }
        });
        StatChanges sc;
        sc.flatMeleeRangeCritChance = 0.1;
        sc.flatForceTechCritChance = 0.1;
        ws->setStatChanges(sc);
        return ws;
    }
    case dirty_fighting_shrap_bomb: {
        auto info = detail::getDefaultAbilityInfo(id);
        info.coefficients[0].multiplier += 0.05; // Bombastic (40)
        auto sb = std::unique_ptr<DOT>(MakeOnAbilityHitDot(id, info, detail::getTickOnWoundingShotsLambda()));
        sb->setDoubleTickChance(0.1); // Gushing Wounds (64)
        info.coefficients[0].isAreaOfEffect = true;
        auto abl = detail::createDotAbility(info, std::move(sb));
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(detail::getGenericDebuff(debuff_assailable)));
        return abl;
    }
    case gunslinger_vital_shot: {
        auto info = detail::getDefaultAbilityInfo(id);
        info.nTicks += 2;
        auto dot = std::unique_ptr<DOT>(MakeOnAbilityHitDot(id, info, detail::getTickOnWoundingShotsLambda()));
        dot->setDoubleTickChance(0.1); // Mortal Wound (32)
        auto abl = detail::createDotAbility(std::move(dot));
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(detail::getGenericDebuff(debuff_marked)));
        return abl;
    }
    case dirty_fighting_hemorraghing_blast: {
        auto info = detail::getDefaultAbilityInfo(id);
        for (auto &c : info.coefficients) {
            c.multiplier += 0.05 + getExploitedWeakness() * 0.5;
        }
        auto abl = std::make_shared<Ability>(id, info);
        auto HemoDebuff = std::unique_ptr<Debuff>(MakeOnAbilityHitDebuff(
            "Hemo Blast", id,
            [=](DamageHits &hits, const Second &, const TargetPtr &source, const TargetPtr &target,
                const Debuff &debuff) -> DamageHits {
                int procCount = 0;
                for (auto &&hit : hits) {
                    if (hit.dt == DamageType::Internal && source->getId() == debuff.getSource()->getId()) {
                        ++procCount;
                    }
                }
                if (procCount == 0)
                    return {};
                DamageHits ret;
                auto fs = getAllFinalStats(*abl, source, target);
                for (int ii = 0; ii < procCount; ++ii) {
                    auto newHits = getHits(*abl, fs, target);
                    ret.insert(ret.end(), newHits.begin(), newHits.end());
                }
                return ret;
            }));
        HemoDebuff->setDuration(Second(10));
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(std::move(HemoDebuff)));
        auto bloodyMayhemDebuff = std::make_unique<detail::BloodyMayhem>();
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(std::move(bloodyMayhemDebuff)));
        abl->setCooldown(Second(15));
        return abl;
    }
    default:
        return Gunslinger::getAbilityInternal(id);
    }
}

} // namespace Simulator
