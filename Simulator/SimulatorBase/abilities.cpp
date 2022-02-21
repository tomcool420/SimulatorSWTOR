#include "abilities.h"
#include "constants.h"
#include "utility.h"
namespace Simulator {
void from_json(const nlohmann::json &json, AbilityCoefficients &coeffs) {
    if (json.contains("Coefficient")) {
        coeffs.coefficient = json["Coefficient"].get<double>();
    }
    if (json.contains("Amount Modifier Percent")) {
        coeffs.AmountModifierPercent = json["Amount Modifier Percent"].get<double>();
    }
    if (json.contains("Standard Health Percent Min")) {
        coeffs.StandardHealthPercentMin = json["Standard Health Percent Min"].get<double>();
    }
    if (json.contains("Standard Health Percent Max")) {
        coeffs.StandardHealthPercentMax = json["Standard Health Percent Max"].get<double>();
    }
    if (json.contains("Damage Type")) {
        coeffs.damageType = Simulator::DamageType(json["Damage Type"].get<int>() - 1);
    }
    if (json.contains("Offhand Hit")) {
        coeffs.isOffhandHit = json["Offhand Hit"].get<bool>();
    }
    if (json.contains("Area Of Effect")) {
        coeffs.isOffhandHit = json["Area Of Effect"].get<bool>();
    }
}
void from_json(const nlohmann::json &json, AllAbilityCoefficient &allcoeffs) {
    CHECK(json.is_array());
    for (auto &&dat : json) {
        AbilityCoefficients c;
        from_json(dat, c);
        allcoeffs.push_back(c);
    }
}
void from_json(const nlohmann::json &json, AbilityInfo &info) {
    if (json.contains("coefficients")) {
        from_json(json["coefficients"], info.coefficients);
    }
    if (json.contains("Number of Ticks")) {
        info.nTicks = json["Number of Ticks"].get<int>();
    }
    if (json.contains("Time")) {
        info.time = Second(json["Time"].get<double>());
    }
    if (json.contains("Initial Tick")) {
        info.extraInitialTick = json["Initial Tick"].get<bool>();
    }
}
} // namespace Simulator
namespace Simulator::detail {
AbilityInfo getAbilityFromJson(const nlohmann::json &json) { return json.get<AbilityInfo>(); }
AbilityInfo getDefaultAbilityInfo(AbilityId id) {
    switch (id) {
    case dirty_fighting_dirty_blast: {
        AbilityCoefficients coeffsMH{1.12, 0.112, 0.112, -0.25};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, -0.25};
        coeffOH.isOffhandHit = true;
        AbilityCoefficients coeffInternalHit{0.5, 0.05, 0.05, 0.0, DamageType::Internal, false};
        AbilityInfo info{{coeffsMH, coeffOH, coeffInternalHit}, AbilityCastType::Cast};
        info.energyCost = 15;
        info.travelSpeed = 10;
        return info;
    }
    case dirty_fighting_shrap_bomb: {
        auto info = AbilityInfo{AllAbilityCoefficient{{0.27, 0.027, 0.027, 0.0, DamageType::Internal, true, false}},
                                AbilityCastType::Instant, Second(3), 8};
        info.energyCost = 10;
        return info;
    }
    case dirty_fighting_exploited_weakness: {
        return AbilityInfo{AllAbilityCoefficient{{0.112, 0.0112, 0.0112, 0, DamageType::Internal, true, false}},
                           AbilityCastType::Instant, Second(3), 6};
    }
    case dirty_fighting_wounding_shots: {
        AbilityCoefficients coeffsMH{0.49, 0.049, 0.049, -0.67};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, -0.67};
        coeffOH.isOffhandHit = true;
        auto info = AbilityInfo{{coeffsMH, coeffOH}, AbilityCastType::Channeled, Second(1.0), 4};
        info.energyCost = 6;
        info.cooldownTime = Second(9);
        return info;
    }
    case dirty_fighting_hemorraghing_blast: {
        AbilityCoefficients coeffsMH{0.17, 0.017, 0.017, -0.89};
        AbilityCoefficients coeffsOH{0.0, 0.0, 0.0, -0.89};
        coeffsOH.isOffhandHit = true;
        auto info = AbilityInfo{{coeffsMH, coeffsOH}};
        info.energyCost = 5;
        info.cooldownTime = Second(18);
        return info;
    }
    case gunslinger_vital_shot: {
        return AbilityInfo{AllAbilityCoefficient{{0.3075, 0.03075, 0.03075, 0, DamageType::Internal, true, false}},
                           AbilityCastType::Instant, Second(3), 6};
    }
    case gunslinger_illegal_mods: {
        AbilityInfo info{{}, AbilityCastType::OffGCD};
        info.cooldownTime = Second(120);
        return info;
    }
    case gunslinger_hunker_down: {
        AbilityInfo info{{}, AbilityCastType::OffGCD};
        info.cooldownTime = Second{60};
        info.ignoresAlacrity = true;
        return info;
    }
    case gunslinger_smugglers_luck: {
        AbilityInfo info{{}, AbilityCastType::OffGCD};
        info.cooldownTime = Second(60);
        return info;
    }
    case gunslinger_quickdraw: {
        AbilityCoefficients coeffMH{2.51, 0.251, 0.251, 0.67};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, 0.67};
        coeffOH.isOffhandHit = true;
        auto info = AbilityInfo{{coeffMH, coeffOH}};
        info.energyCost = 15;
        info.cooldownTime = Second(12);
        return info;
    }
    case sharpshooter_trickshot: {
        AbilityCoefficients coeffMH{1.917, 0.192, 0.192, 0.28};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, 0.28};
        coeffOH.isOffhandHit = true;
        auto info = AbilityInfo{{coeffMH, coeffOH}};
        info.energyCost = 5;
        info.cooldownTime = Second(9);
        return info;
    }
    case sharpshooter_aimed_shot: {
        AbilityCoefficients coeffMH{3.684, 0.37, 0.37, 1.36};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, 1.36};
        coeffOH.isOffhandHit = true;
        auto info = AbilityInfo{{coeffMH, coeffOH}};
        info.energyCost = 15;
        info.cooldownTime = Second(12);
        return info;
    }
    case sharpshooter_penetrating_blasts: {
        AbilityCoefficients coeffsMH{0.8, 0.08, 0.08, -0.47};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, -0.47};
        coeffOH.isOffhandHit = true;
        auto info = AbilityInfo{{coeffsMH, coeffOH}, AbilityCastType::Channeled, Second(1.0), 4};
        info.energyCost = 6;
        info.nTicks = 5;
        info.time = Second(0.4);
        info.cooldownTime = Second(15);
        return info;
    }
    case gunslinger_speed_shot: {
        AbilityCoefficients coeffMH{0.93, 0.093, 0.093, -0.38};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, -0.38};
        coeffOH.isOffhandHit = true;
        AbilityInfo info{{coeffMH, coeffOH}};
        info.type = AbilityCastType::Channeled;
        info.nTicks = 4;
        info.time = Second(1.0);
        info.energyCost = 5;
        return info;
    }
    case gunslinger_charged_burst: {
        AbilityCoefficients coeffMH{1.74, 0.174, 0.174, 0.16, DamageType::Weapon};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, 0.16, DamageType::Weapon};
        coeffOH.isOffhandHit = true;
        auto info = AbilityInfo{{coeffMH, coeffOH}};
        info.energyCost = 20;
        info.cooldownTime = Second(0);
        return info;
    }
    case gunslinger_xs_freighter_flyby: {
        AbilityCoefficients coeff{0.7, 0.07, 0.07, 0.0, DamageType::Elemental};
        AbilityInfo info{{coeff}};
        info.nTicks = 3;
        info.extraInitialTick = false;
        info.time = Second(3);
        return info;
    }

    case smuggler_flurry_of_bolts: {
        AbilityCoefficients coeffMH{1.0, 0.0, 0.0, 0.0};
        AbilityCoefficients coeffOH{0.0, 0.0, 0.0, 0.0};
        coeffOH.isOffhandHit = true;
        return AbilityInfo{{coeffMH, coeffOH}};
    }

    case tactics_gut: {
        return AbilityInfo{{AbilityCoefficients{0.95, 0.075, 0.115, 0.0, DamageType::Kinetic, false, false}}};
    }
    case tactics_gut_dot: {
        auto ac = AllAbilityCoefficient{{0.25, 0.025, 0.025, 0, DamageType::Internal, true, false}};
        auto ai = AbilityInfo{ac};
        ai.extraInitialTick = true;
        ai.nTicks = 6;
        ai.time = Second(3);
        return ai;
    }
    case tactics_tactical_surge: {
        return AbilityInfo{{{1.72, 0.152, 0.192, 0.0, DamageType::Kinetic, false, false}}};
    }
    case tactics_cell_burst: {
        return AbilityInfo{{{0.84, 0.079, 0.089, 0.0, DamageType::Energy}}};
    }

    case vanguard_stockstrike: {
        return AbilityInfo{{{1.77, 0.158, 0.198, 0.0, DamageType::Kinetic, false, false}}};
    }

    case trooper_high_impact_bolt: {
        return AbilityInfo{{{1.97, 0.197, 0.197, 0.31, DamageType::Weapon}}};
    }

    case consular_double_strike: {
        return AbilityInfo{
            {{0.72, 0.072, 0.072, -0.52, DamageType::Weapon}, {0.72, 0.072, 0.072, -0.52, DamageType::Weapon}}};
    }
    case infiltration_clairvoyant_strike: {
        return AbilityInfo{
            {{0.75, 0.075, 0.075, -0.5, DamageType::Weapon}, {0.75, 0.075, 0.075, -0.5, DamageType::Weapon}}};
    }
    case infiltration_force_breach: {
        return AbilityInfo{{{0.73, 0.053, 0.093, 0.0, DamageType::Internal}}};
    }
    case infiltration_psychokinetic_blast: {
        return AbilityInfo{{{1.84, 0.164, 0.204, 0.0, DamageType::Kinetic}}};
    }
    case infiltration_psychokinetic_blast_upheaval: {
        return AbilityInfo{{{0.92, 0.072, 0.112, 0.0, DamageType::Kinetic}}};
    }
    case infiltration_shadow_technique: {
        return AbilityInfo{{{0.585, 0.0585, 0.0585, 0.0, DamageType::Internal}}};
    }
    case shadow_shadow_strike: {
        return AbilityInfo{{{2.33, 0.233, 0.233, 0.55, DamageType::Weapon}}};
    }
    case shadow_spinning_strike: {
        return AbilityInfo{{{2.54, 0.254, 0.254, 0.7, DamageType::Weapon}}};
    }
    case consular_saber_strike: {
        return AbilityInfo{{{0.33, 0.0, 0.0, -0.66, DamageType::Weapon}, {0.66, 0.0, 0.0, -0.33, DamageType::Weapon}}};
    }
    case infiltration_vaulting_slash: {
        return AbilityInfo{{{2.75, .275, 0.275, 0.83, DamageType::Weapon}}};
    }
    }
    CHECK(false, "Coefficients for ability {} are not implemented yet", id);
    return AbilityInfo{};
}
} // namespace Simulator::detail
