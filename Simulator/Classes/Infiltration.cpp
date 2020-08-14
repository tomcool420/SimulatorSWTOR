#include "Infiltration.h"
#include "Shadow.h"
#include "Simulator/SimulatorBase/abilities.h"
#include <Simulator/SimulatorBase/AbilityBuff.h>
#include <Simulator/SimulatorBase/ActiveStackingBuff.h>
#include <Simulator/SimulatorBase/Target.h>
namespace Simulator {
namespace detail {
class ShadowTechnique : public Buff {
  public:
    ShadowTechnique() {
        setMaxStacks(3);
        setStackDuration(Second(30));
        setId(infiltration_shadow_technique);
    }
    DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &player,
                            const TargetPtr &target) override {
        if (_nextAvailableProc && time < _nextAvailableProc.value())
            return {};
        for (auto &&hit : hits) {
            if (hit.weapon && getRandomValue(0.0, 1.0) > 0.5 &&
                (!_nextAvailableProc || time > _nextAvailableProc.value())) {
                auto infos = detail::getDefaultAbilityInfo(infiltration_shadow_technique);
                Ability st(infiltration_shadow_technique, infos);
                auto fs = getAllFinalStats(st, player, target);
                auto ala = fs[0].alacrity;
                _nextAvailableProc = time + Second(6.0) / (1.0 + ala);
                setCurrentStacks(std::min(getMaxStacks(), getCurrentStacks() + 1), time);
                return getHits(st, fs, target);
            } else if (hit.id == infiltration_force_breach && !hit.miss) {
                setCurrentStacks(0, time);
            }
        }
        return {};
    }
    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr & /*target*/) const override {
        if (ability.getId() == infiltration_force_breach) {
            for (auto &fs : fstats) {
                fs.multiplier += std::max<double>(0.0, getCurrentStacks() - 1);
            }
        }
    }

  private:
    std::optional<Second> _nextAvailableProc = std::nullopt;
};

class Clairvoyance : public Buff {
  public:
    Clairvoyance() : Buff() {
        setId(infiltration_clairvoyance);
        setStackDuration(Second(10));
        setMaxStacks(2);
    }
    DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &player,
                            const TargetPtr &target) override {
        for (auto &&hit : hits) {
            if (hit.id == infiltration_clairvoyant_strike && !hit.miss) {
                setCurrentStacks(getCurrentStacks() + 1, time);
                break;
            }
            if (hit.id == infiltration_psychokinetic_blast && !hit.miss) {
                if (getCurrentStacks() == 2 || (getCurrentStacks() == 1 && getRandomValue(0.0, 1.0) > 0.5)) {
                    auto b = player->getBuff<ShadowTechnique>(infiltration_shadow_technique);
                    b->setCurrentStacks(b->getCurrentStacks() + 1, time);
                }
            }
        }
        return {};
    }
};
class InfiltrationTactics : public Buff {
  public:
    InfiltrationTactics() {
        setMaxStacks(1);
        setStackDuration(Second(20));
        setId(infiltration_infiltration_tactics);
    }
    DamageHits onAbilityHit(DamageHits &hits, const Second &time, const TargetPtr &player,
                            const TargetPtr &target) override {
        for (auto &&hit : hits) {
            if (getCurrentStacks() > 0 && hit.id == shadow_shadow_strike && !hit.miss) {
                setCurrentStacks(0, time);
                break;
            }
            if (hit.weapon || hit.direct) {
                if (!_nextAvailableProc || time >= _nextAvailableProc) {
                    setCurrentStacks(1, time);
                    auto afs = getAllFinalStats({}, player, nullptr);
                    _nextAvailableProc = Second(20) / (1.0 + afs[0].alacrity);
                }
            }
        }
        return {};
    }
    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr & /*target*/) const override {
        if (getCurrentStacks() && ability.getId() == shadow_shadow_strike) {
            for (auto &fs : fstats) {
                fs.multiplier += 0.2;
            }
        }
    }

  private:
    std::optional<Second> _nextAvailableProc = std::nullopt;
};

class Judgement : public Buff {
  public:
    void apply(const Ability &, AllStatChanges &fstats, const TargetPtr &target) const override {
        if (target && target->getCurrentHealth() < target->getMaxHealth() * 0.3) {
            for (auto &fs : fstats) {
                fs.multiplier += 0.03;
            }
        }
    }
};

class DeepImpact : public RawSheetBuff {
  public:
    DeepImpact() {
        setId(infiltration_deep_impact);
        setIds({infiltration_psychokinetic_blast, infiltration_force_breach});
        StatChanges sc;
        sc.flatForceTechCriticalMultiplierBonus = 0.2;
        sc.flatMeleeRangeCriticalMultiplierBonus = 0.2;
        setStatChanges(sc);
    }
};

class WhirlingEdge : public Buff {
  public:
    WhirlingEdge() { setId(infiltration_whirling_edge); }
    void apply(const Ability &abl, AllStatChanges &fstats, const TargetPtr &target) const override {
        for (auto &fs : fstats) {
            fs.armorPen += 0.1;
        }
        if (abl.getId() == shadow_whirling_blow) {
            for (auto &fs : fstats) {
                fs.flatMeleeRangeCriticalMultiplierBonus += 0.3;
                fs.flatMeleeRangeCritChance += 0.15;
            }
        }
    }
};
} // namespace detail
std::vector<BuffPtr> Infiltration::getStaticBuffs() {
    auto b = Shadow::getStaticBuffs();
    b.push_back(std::make_unique<detail::ShadowTechnique>());
    b.push_back(std::make_unique<detail::Clairvoyance>());
    b.push_back(std::make_unique<detail::WhirlingEdge>());
    b.push_back(std::make_unique<detail::DeepImpact>());
    b.push_back(std::make_unique<detail::Judgement>());
    b.push_back(std::make_unique<detail::InfiltrationTactics>());

    return b;
}

AbilityPtr Infiltration::getAbilityInternal(AbilityId id) {
    switch (id) {
    case infiltration_clairvoyant_strike: {
        auto infos = detail::getDefaultAbilityInfo(id);
        for (auto &ii : infos.coefficients) {
            ii.multiplier += 0.2; // Prevailing Strikes (56)
        }
        return std::make_shared<Ability>(id, infos);
    }
    case infiltration_vaulting_slash: {
        auto infos = detail::getDefaultAbilityInfo(id);
        for (auto &ii : infos.coefficients) {
            ii.multiplier += 0.2; // Prevailing Strikes (56)
        }
        return std::make_shared<Ability>(id, infos);
    }
    case infiltration_psychokinetic_blast: {
        auto infos = detail::getDefaultAbilityInfo(id);
        return std::make_shared<Ability>(id, infos);
    }
    case infiltration_force_breach: {
        auto infos = detail::getDefaultAbilityInfo(id);
        return std::make_shared<Ability>(id, infos);
    }
    }

    return Shadow::getAbilityInternal(id);
}

} // namespace Simulator