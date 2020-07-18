#include "helpers.h"

namespace Simulator {
std::vector<BuffPtr> getTacticsSheetBuffs() {
    std::vector<BuffPtr> ret;
    ret.push_back(std::make_unique<RawSheetBuff>("High Friction Bolts",
                                                 std::vector<uint64_t>{
                                                     2021194429628416 // high Impact Bolt
                                                 },
                                                 0.0, 0.0, 0.0, 0.3));
    ret.push_back(std::make_unique<RawSheetBuff>("Serrated Blades",
                                                 std::vector<uint64_t>{
                                                     2029878853501184 // Gut Dot
                                                 },
                                                 0.15, 0.0, 0.0, 0.0));
    ret.push_back(std::make_unique<RawSheetBuff>("Critical Recharge", std::vector<uint64_t>{}, 0.0, 0.02, 0.0, 0.0));
    ret.push_back(std::make_unique<RawSheetBuff>("Riot Augs",
                                                 std::vector<uint64_t>{
                                                     801367882989568,  // stockstrike
                                                     2021194429628416, // high Impact Bolt
                                                     3393260387041280, // tactical surge
                                                     3393354876321792, // cell burst
                                                     3393277566910464, // assault plastique
                                                     2029878853501184, // Gut Dot
                                                 },
                                                 0.1, 0.0, 0.0, 0.0));

    ret.push_back(std::make_unique<RawSheetBuff>("Havoc Training",
                                                 std::vector<uint64_t>{
                                                     801367882989568,  // stockstrike
                                                     2021194429628416, // high Impact Bolt
                                                     3393260387041280, // tactical surge
                                                     3393354876321792, // cell burst
                                                     3393277566910464, // assault plastique
        2029878853501184, // Gut Dot

                                                 },
                                                 0.0, 0.0, 0.1, 0.0));

    ret.push_back(std::make_unique<RawSheetBuff>("Focused Impact",
                                                 std::vector<uint64_t>{
                                                     2021194429628416, // high Impact Bolt
                                                 },
                                                 0.0, 0.0, 0.0, 0.6));

    ret.push_back(std::make_unique<RawSheetBuff>("Focused Impact",
                                                 std::vector<uint64_t>{
                                                     801367882989568, // stockstrike
                                                 },
                                                 0.1, 0.0, 0.0, 0.0));

    ret.push_back(std::make_unique<DamageTypeBuff>("High Energy Gas Cell",
                                                   std::vector<DamageType>{DamageType::Kinetic, // stockstrike
                                                                           DamageType::Energy,  // stockstrike
                                                                           DamageType::Weapon},
                                                   0.07, 0.0, 0.0, 0.0));

    return ret;
}

BuffPtr getDefaultStatsBuffPtr(){
    StatChanges sb = getDefaultStatsBuffs();
    return std::make_unique<RawSheetBuff>("Class Buffs",std::vector<AbilityId>{},sb);
}

StatChanges getDefaultStatsBuffs(bool twoPiece ) {
    StatChanges sb;
    sb.masteryMultiplierBonus = 0.05 + (twoPiece ? 0.02:0.00);       // Set bonus + force valor;
    sb.flatMeleeRangeCritChance = 0.06;           // companion + lucky shots;
    sb.flatForceTechCritChance = 0.06;           // companion + lucky shots;
    sb.flatMeleeRangeCriticalMultiplierBonus = 0.01; // companion
    sb.flatForceTechCriticalMultiplierBonus = 0.01; // companion

    sb.bonusDamageMultiplier = 0.05;       // force might
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
