#include "helpers.h"
#include "../Ability.h"
#include "../ConditionalApplyDebuff.h"
#include "../constants.h"
#include "../types.h"

namespace Simulator {
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
    case tactics_gut: {
        auto abl = std::make_shared<Ability>(tactics_gut, 0.95, 0.075, 0.115, 0.0, DamageType::Kinetic, false, false);
        auto gut_dot = std::make_unique<DOT>(tactics_gut_dot, 0.25, 0.025, 0.025, 0, DamageType::Internal, true, false,
                                             7, Second(3), true);
        auto conditionalGutDot = std::make_shared<ConditionalApplyDebuff>(std::move(gut_dot));
        abl->addOnHitAction(conditionalGutDot);
        return abl;
    }
    case dirty_fighting_hemorraghing_blast: {
        AbilityCoefficients coeffs;
        coeffs.coefficient = 0.17;
        coeffs.StandardHealthPercentMax = 0.017;
        coeffs.StandardHealthPercentMin = 0.017;
        coeffs.AmountModifierPercent = -0.89;
        coeffs.damageType = DamageType::Weapon;
        auto abl = std::make_shared<Ability>(dirty_fighting_hemorraghing_blast, coeffs);
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
                auto fs = getFinalStats(*abl, source, target);
                for (int ii = 0; ii < procCount; ++ii) {
                    auto newHits = getHits(*abl, fs, target);
                    ret.insert(ret.end(), newHits.begin(), newHits.end());
                }
                return ret;
            }));
        HemoDebuff->setDuration(Second(15));
        auto conditionalHemoDebuff = std::make_shared<ConditionalApplyDebuff>(std::move(HemoDebuff));
        abl->addOnHitAction(conditionalHemoDebuff);
        return abl;
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

    ret.push_back(std::make_unique<DamageTypeBuff>("High Energy Gas Cell",
                                                   std::vector<DamageType>{DamageType::Kinetic, // stockstrike
                                                                           DamageType::Energy,  // stockstrike
                                                                           DamageType::Weapon},
                                                   0.07, 0.0, 0.0, 0.0));

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
