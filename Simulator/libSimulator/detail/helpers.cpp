#include "helpers.h"
#include "../Ability.h"
#include "../AbilityDebuff.h"
#include "../ConditionalApplyDebuff.h"
#include "../constants.h"
#include "../detail/log.h"
#include "../types.h"
namespace Simulator {
class EntrencedOffsenceBuff : public Buff {
  public:
    EntrencedOffsenceBuff() : Buff() {
        setId(gunslinger_entrenched_offsense);
        setDuration(Second(15));
    };
    [[nodiscard]] virtual DebuffEvents resolveEventsUpToTime(const Second &time, const TargetPtr &) {
        _stacks = std::clamp(static_cast<int>(std::floor((time - getStartTime()).getValue())), 0, 5);
        SIM_INFO("Entrenched Offense is now at {} stacks ", _stacks);
        if (time > getEndTime()) {
            return {{DebuffEventType::Remove}};
        }
        return {};
    }
    virtual void apply(const Ability & /*ability*/, AllStatChanges &fstats, const TargetPtr & /*target*/) const {
        for (auto &&stats : fstats) {
            stats.multiplier += 0.03 * _stacks;
        }
    }
    [[nodiscard]] std::optional<Second> getNextEventTime() const final {
        if (_stacks < 5)
            return getStartTime() + ((_stacks + 1) * Second(1.0));
        return Buff::getNextEventTime();
    }
    [[nodiscard]] Buff *clone() const final { return new EntrencedOffsenceBuff(*this); };

  private:
    int _stacks{0};
};
auto getWoundingShotsLambda() {
    return [](DamageHits &hits, const Second &time, const TargetPtr &source, const TargetPtr &target,
              DOT &dot) -> DamageHits {
        bool hitWoundingShots = false;
        for (auto &&hit : hits) {
            if (hit.id == dirty_fighting_wounding_shots && source->getId() == dot.getSource()->getId()) {
                hitWoundingShots = true;
                break;
            }
        }
        if (!hitWoundingShots)
            return {};
        return dot.tick(target, time, true);
    };
}
DOTPtr getDot(AbilityId id) {
    switch (id) {
    case dirty_fighting_shrap_bomb: {
        AbilityCoefficients coeffs{0.27, 0.027, 0.027, 0.0, DamageType::Internal, true, false};
        coeffs.isBleedDamage = true;
        coeffs.multiplier = 0.05; // bombastic (40)
        auto sb = std::unique_ptr<DOT>(
            MakeOnAbilityHitDot(dirty_fighting_shrap_bomb, coeffs, 8, Second(3), false, getWoundingShotsLambda()));
        sb->setDoubleTickChance(0.1); // gushing wounds (64)
        return sb;
    }
    case gunslinger_vital_shot: {
        AbilityCoefficients coeffs{0.3075, 0.03075, 0.03075, 0, DamageType::Internal, true, false};
        coeffs.isBleedDamage = true;
        auto vs = std::unique_ptr<DOT>(
            MakeOnAbilityHitDot(gunslinger_vital_shot, coeffs, 8, Second(3), false, getWoundingShotsLambda()));
        vs->setDoubleTickChance(0.1); // Mortal Wound (32)
        return vs;
    }
    case dirty_fighting_exploited_weakness: {
        AbilityCoefficients coeffs{0.112, 0.0112, 0.0112, 0, DamageType::Internal, true, false};
        coeffs.isBleedDamage = true;
        auto vs = std::unique_ptr<DOT>(MakeOnAbilityHitDot(dirty_fighting_exploited_weakness, coeffs, 6, Second(3),
                                                           false, getWoundingShotsLambda()));
        vs->setTickOnRefresh(true);
        return vs;
    }
    default:
        return nullptr;
    }
}
AbilityPtr createDotAbility(AbilityId id) {
    auto vs = getDot(id);
    CHECK(vs);
    auto abl = std::make_shared<Ability>(id, vs->getAbility().getInfo());
    abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(std::move(vs)));
    return abl;
}
AbilityPtr getAbility(AbilityId id) {
    switch (id) {
    case vanguard_stockstrike:
        return std::make_shared<Ability>(vanguard_stockstrike, 1.77, 0.158, 0.198, 0.0, DamageType::Kinetic, false,
                                         false);
    case tactics_tactical_surge:
        return std::make_shared<Ability>(tactics_tactical_surge, 1.72, 0.152, 0.192, 0.0, DamageType::Kinetic, false,
                                         false);
    case trooper_high_impact_bolt:
        return std::make_shared<Ability>(trooper_high_impact_bolt, 1.97, 0.197, 0.197, 0.31, DamageType::Weapon, false,
                                         false);
    case tactics_cell_burst: {
        AbilityCoefficients coeffs{0.84, 0.079, 0.089, 0.0, DamageType::Energy};
        return std::make_shared<Ability>(tactics_cell_burst, AbilityInfo{{coeffs}});
    }

    case tactics_gut: {
        auto abl = std::make_shared<Ability>(tactics_gut, 0.95, 0.075, 0.115, 0.0, DamageType::Kinetic, false, false);
        auto gut_dot = std::make_unique<DOT>(tactics_gut_dot, 0.25, 0.025, 0.025, 0, DamageType::Internal, true, false,
                                             7, Second(3), true);
        auto conditionalGutDot = std::make_shared<ConditionalApplyDebuff>(std::move(gut_dot));
        abl->addOnHitAction(conditionalGutDot);
        return abl;
    }
    case dirty_fighting_hemorraghing_blast: {
        AbilityCoefficients coeffsMH{0.17, 0.017, 0.017, -0.89};
        AbilityCoefficients coeffsOH{0.0, 0.0, 0.0, -0.89};
        coeffsOH.isOffhandHit = true;
        coeffsOH.multiplier = 0.55;
        coeffsMH.multiplier = 0.55;
        auto abl = std::make_shared<Ability>(dirty_fighting_hemorraghing_blast, AbilityInfo{{coeffsMH, coeffsOH}});
        auto HemoDebuff = std::unique_ptr<Debuff>(MakeOnAbilityHitDebuff(
            "Hemo Blast", dirty_fighting_hemorraghing_blast,
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
        auto bloodyMayhemDebuff = std::make_unique<BloodyMayhemDebuff>();
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(std::move(bloodyMayhemDebuff)));

        return abl;
    }
    case dirty_fighting_shrap_bomb: {
        auto vs = getDot(id);
        auto info = vs->getAbility().getInfo();
        for (auto &&i : info.coefficients) {
            i.isAreaOfEffect = true;
        }
        auto abl = std::make_shared<Ability>(id, info);
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(std::move(vs)));

        auto assailable = getDebuff(debuff_assailable);
        CHECK(assailable);
        auto cAss = std::make_shared<ConditionalApplyDebuff>(std::move(assailable));
        abl->addOnHitAction(cAss);
        return abl;
    }
    case gunslinger_vital_shot: {
        auto abl = createDotAbility(gunslinger_vital_shot);
        auto marked = getDebuff(debuff_marked);
        CHECK(marked);
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(std::move(marked)));
        return abl;
    }
    case dirty_fighting_dirty_blast: {
        AbilityCoefficients coeffsMH{1.12, 0.112, 0.112, -0.25};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, -0.25};
        coeffOH.isOffhandHit = true;
        coeffsMH.multiplier = 0.1;
        coeffOH.multiplier = 0.1;
        AbilityCoefficients coeffInternalHit{0.5, 0.05, 0.05, 0.0, DamageType::Internal, false};
        coeffInternalHit.multiplier = 0.1;
        AbilityInfo info{{coeffsMH, coeffOH, coeffInternalHit}, AbilityCastType::Cast};
        auto abl = std::make_shared<Ability>(dirty_fighting_dirty_blast, std::move(info));
        abl->addOnHitAction(std::make_shared<ConditionalApplyDebuff>(getDot(dirty_fighting_exploited_weakness)));
        return abl;
    }
    case dirty_fighting_wounding_shots: {
        AbilityCoefficients coeffsMH{0.49, 0.049, 0.049, -0.67};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, -0.67};
        coeffOH.isOffhandHit = true;
        AbilityInfo info{{coeffsMH, coeffOH}, AbilityCastType::Channeled, Second(1.0), 4};
        auto ws = std::make_shared<Ability>(dirty_fighting_wounding_shots, std::move(info));
        ws->addOnEndAction([](const TargetPtr &source, const TargetPtr &, const Second &time) {
            if (source->getBuff<Buff>(gunslinger_smugglers_luck)) {
                source->removeBuff(gunslinger_smugglers_luck, time);
            }
        });
        return ws;
    }
    case gunslinger_illegal_mods: {
        AbilityInfo info{{}, AbilityCastType::OffGCD};
        StatChanges sc;
        sc.armorPen = 0.15;
        sc.flatForceTechAccuracy = 0.3;
        sc.flatMeleeRangeAccuracy = 0.3;
        auto ass = std::make_unique<RawSheetBuff>("Illegal Mods", AbilityIds{}, sc);
        ass->setDuration(Second{10});
        ass->setId(gunslinger_illegal_mods);
        auto abl = std::make_shared<Ability>(gunslinger_illegal_mods, info);
        abl->addOnHitAction(std::make_shared<ConditionalApplyBuff>(std::move(ass), false));
        return abl;
    }
    case gunslinger_quickdraw: {
        AbilityCoefficients coeffMH{2.51, 0.251, 0.251, 0.67};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, 0.67};
        coeffOH.isOffhandHit = true;
        return std::make_shared<Ability>(gunslinger_quickdraw, AbilityInfo{{coeffMH, coeffOH}});
    }
    case gunslinger_smugglers_luck: {
        AbilityInfo info{{}, AbilityCastType::OffGCD};
        StatChanges sc;
        sc.flatForceTechCritChance += 1.0;
        sc.flatMeleeRangeCritChance += 1.0;
        auto ass = std::make_unique<RawSheetBuff>("Smuggler's Luck", AbilityIds{dirty_fighting_wounding_shots}, sc);
        ass->setDuration(Second{20});
        ass->setId(gunslinger_smugglers_luck);
        auto abl = std::make_shared<Ability>(gunslinger_smugglers_luck, info);
        abl->addOnHitAction(std::make_shared<ConditionalApplyBuff>(std::move(ass), false));
        return abl;
    }
    case gunslinger_hunker_down: {
        AbilityInfo info{{}, AbilityCastType::OffGCD};

        auto abl = std::make_shared<Ability>(gunslinger_smugglers_luck, info);
        abl->addOnHitAction(std::make_shared<ConditionalApplyBuff>(std::make_unique<EntrencedOffsenceBuff>(), false));
        return abl;
    }
    default:
        return nullptr;
    }
}

DebuffPtr getDebuff(AbilityId id) {
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
    default:
        break;
    }
    return nullptr;
}

BuffPtr getBuff(AbilityId id) {
    switch (id) {
    case test_debuff: {
        StatChanges sc;
        auto ass = std::make_unique<RawSheetBuff>("Test Buff", AbilityIds{}, sc);
        ass->setId(test_buff);
        ass->setDuration(Second(20));
        return ass;
    }
    default:
        return nullptr;
    }
}
std::vector<BuffPtr> getTacticsSheetBuffs() {
    std::vector<BuffPtr> ret;
    ret.push_back(std::make_unique<RawSheetBuff>("High Friction Bolts",
                                                 std::vector<uint64_t>{
                                                     trooper_high_impact_bolt // high Impact Bolt
                                                 },
                                                 0.0, 0.0, 0.0, 0.3));
    ret.push_back(std::make_unique<RawSheetBuff>("Serrated Blades",
                                                 std::vector<uint64_t>{
                                                     tactics_gut_dot // Gut Dot
                                                 },
                                                 0.15, 0.0, 0.0, 0.0));
    ret.push_back(std::make_unique<RawSheetBuff>("Critical Recharge", std::vector<uint64_t>{}, 0.0, 0.02, 0.0, 0.0));
    ret.push_back(std::make_unique<RawSheetBuff>("Riot Augs",
                                                 std::vector<uint64_t>{
                                                     vanguard_stockstrike,      // stockstrike
                                                     trooper_high_impact_bolt,  // high Impact Bolt
                                                     tactics_tactical_surge,    // tactical surge
                                                     tactics_cell_burst,        // cell burst
                                                     tactics_assault_plastique, // assault plastique
                                                     tactics_gut_dot,           // Gut Dot
                                                 },
                                                 0.1, 0.0, 0.0, 0.0));

    ret.push_back(std::make_unique<RawSheetBuff>("Havoc Training",
                                                 std::vector<uint64_t>{
                                                     vanguard_stockstrike,      // stockstrike
                                                     trooper_high_impact_bolt,  // high Impact Bolt
                                                     tactics_tactical_surge,    // tactical surge
                                                     tactics_cell_burst,        // cell burst
                                                     tactics_assault_plastique, // assault plastique
                                                     tactics_gut_dot,           // Gut Dot

                                                 },
                                                 0.0, 0.0, 0.1, 0.0));

    ret.push_back(std::make_unique<RawSheetBuff>("Focused Impact",
                                                 std::vector<uint64_t>{
                                                     trooper_high_impact_bolt, // high Impact Bolt
                                                 },
                                                 0.0, 0.0, 0.0, 0.6));

    ret.push_back(std::make_unique<RawSheetBuff>("Focused Impact",
                                                 std::vector<uint64_t>{
                                                     vanguard_stockstrike, // stockstrike
                                                 },
                                                 0.1, 0.0, 0.0, 0.0));

    ret.push_back(std::make_unique<DamageTypeBuff>(
        "High Energy Gas Cell",
        std::vector<DamageType>{
            //                                                                           DamageType::Kinetic, //
            //                                                                           stockstrike
            //                                                                           DamageType::Energy,  //
            //                                                                           stockstrike
            DamageType::Weapon},
        0.07, 0.0, 0.0, 0.0));
    ret.push_back(std::make_unique<EnergyLodeBuff>());

    return ret;
}

BuffPtr getDefaultStatsBuffPtr(bool twopiece, bool masteryBuffs) {
    StatChanges sb = getDefaultStatsBuffs(twopiece, masteryBuffs);
    return std::make_unique<RawSheetBuff>("Class Buffs", std::vector<AbilityId>{}, sb);
}

StatChanges getDefaultStatsBuffs(bool twoPiece, bool masteryBuffs) {
    StatChanges sb;
    sb.masteryMultiplierBonus = masteryBuffs ? 0.05 + (twoPiece ? 0.02 : 0.00) : 0.0; // Set bonus + force valor;
    sb.flatMeleeRangeCritChance = 0.06;                                               // companion + lucky shots;
    sb.flatForceTechCritChance = 0.06;                                                // companion + lucky shots;
    sb.flatMeleeRangeCriticalMultiplierBonus = 0.01;                                  // companion
    sb.flatForceTechCriticalMultiplierBonus = 0.01;                                   // companion

    sb.bonusDamageMultiplier = 0.05; // force might
    return sb;
}

RawStats getDefaultStats() {
    RawStats rs;
    rs.master = Mastery(4585);
    rs.power = Power(2309);
    rs.criticalRating = CriticalRating(462);
    rs.alacrityRating = AlacrityRating(0);
    rs.accuracyRating = AccuracyRating(1557);
    rs.forceTechPower = FTPower(3504);
    rs.weaponDamageMH = {1573.0, 2359.0};
    return rs;
}

} // namespace Simulator
