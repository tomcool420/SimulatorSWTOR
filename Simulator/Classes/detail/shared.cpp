#include "shared.h"
#include "Simulator/SimulatorBase/Ability.h"
#include "Simulator/SimulatorBase/AbilityDebuff.h"
#include "Simulator/SimulatorBase/Debuff.h"
#include "Simulator/SimulatorBase/StatBuff.h"
#include "Simulator/SimulatorBase/constants.h"
namespace Simulator::detail {
class ShatteredDebuff : public Debuff {
  public:
    ShatteredDebuff() : Debuff(debuff_shattered){};
    [[nodiscard]] DamageHits onAbilityHit(DamageHits &hits, const Second & /*time*/, const TargetPtr & /*player*/,
                                          const TargetPtr & /*target*/) final;
    void modifyStats(const Ability &ability, AllStatChanges &fstats,
                     const std::shared_ptr<const Target> &target) const final;
    Debuff *clone() const override { return new ShatteredDebuff(*this); }
};
void ShatteredDebuff::modifyStats(const Ability &ability, AllStatChanges &fstats,
                                  const std::shared_ptr<const Target> &) const {
    for (int ii = 0; ii < fstats.size(); ++ii) {
        auto &c = ability.getCoefficients()[ii];
        auto &s = fstats[ii];
        s.armorDebuff = true;
        s.multiplier += 0.05; // melee, range, force, tech
        if (c.damageType == DamageType::Internal || c.damageType == DamageType::Elemental) {
            s.multiplier += 0.07; // assailable;
        }
    }
}
DamageHits ShatteredDebuff::onAbilityHit(DamageHits &hits, const Second & /*time*/, const TargetPtr & /*player*/,
                                         const TargetPtr & /*target*/) {
    for (auto &&hit : hits) {
        if (hit.aoe) {
            hit.dmg *= 1.1;
        }
    }
    return {};
}

BuffPtr getGenericBuff(AbilityId id) {
    switch (id) {
    case supercharged_celerity: {
        StatChanges sc;
        sc.flatAlacrityBonus = 0.1;
        return nullptr;
    }
    }
    return nullptr;
}

DebuffPtr getGenericDebuff(AbilityId id) {
    switch (id) {
    case debuff_assailable: {
        StatChanges sc;
        sc.multiplier = 0.07;
        auto ass = std::make_unique<RawSheetDebuff>("Assailable", debuff_assailable,
                                                    DamageTypes{DamageType::Internal, DamageType::Elemental}, sc);
        ass->setDuration(Second{45});
        ass->setBlockedByDebuffs({debuff_shattered});
        return ass;
    }
    case debuff_marked: {
        StatChanges sc;
        sc.multiplier = 0.05;
        auto marked = std::make_unique<RawSheetDebuff>("Marked", debuff_marked, DamageTypes{DamageType::Weapon}, sc);
        marked->setDuration(Second{45});
        marked->setBlockedByDebuffs({debuff_shattered});
        return marked;
    }
    case test_debuff: {
        StatChanges sc;
        auto ass = std::make_unique<RawSheetDebuff>("Test Debuff", test_debuff, DamageTypes{}, sc);
        ass->setDuration(Second(20));
        return ass;
    }
    case debuff_shattered: {
        return std::make_unique<ShatteredDebuff>();
    }
    }
    CHECK(false, "Tried to create a debuff with id {} but was not available as a generic debuff", id);
    return nullptr;
}

BuffPtr getDefaultStatsBuffPtr(bool twopiece, bool forceValor) {
    StatChanges sb;
    sb.masteryMultiplierBonus = forceValor * 0.05 + twopiece * 0.02; // Set bonus + force valor;
    sb.flatMeleeRangeCritChance = 0.06;                              // companion + lucky shots;
    sb.flatForceTechCritChance = 0.06;                               // companion + lucky shots;
    sb.flatMeleeRangeCriticalMultiplierBonus = 0.01;                 // companion
    sb.flatForceTechCriticalMultiplierBonus = 0.01;                  // companion

    sb.bonusDamageMultiplier = 0.05; // force might
    return std::make_unique<StatBuff>(sb);
}
} // namespace Simulator::detail
