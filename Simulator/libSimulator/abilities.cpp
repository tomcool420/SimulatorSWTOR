#include "abilities.h"
#include "constants.h"
#include "utility.h"
namespace Simulator{
void from_json(const nlohmann::json & json, AbilityCoefficients &coeffs){
    if(json.contains("Coefficient")){
        coeffs.coefficient= json["Coefficient"].get<double>();
    }
    if(json.contains("Amount Modifier Percent")){
        coeffs.AmountModifierPercent=json["Amount Modifier Percent"].get<double>();
    }
    if(json.contains("Standard Health Percent Min")){
        coeffs.StandardHealthPercentMin=json["Standard Health Percent Min"].get<double>();
    }
    if(json.contains("Standard Health Percent Max")){
        coeffs.StandardHealthPercentMax=json["Standard Health Percent Max"].get<double>();
    }
    if(json.contains("Damage Type")){
        coeffs.damageType=Simulator::DamageType(json["Damage Type"].get<int>()-1);
    }
    if(json.contains("Offhand Hit")){
        coeffs.isOffhandHit=json["Offhand Hit"].get<bool>();
    }
    if(json.contains("Area Of Effect")){
        coeffs.isOffhandHit=json["Area Of Effect"].get<bool>();
    }
}
void from_json(const nlohmann::json & json, AllAbilityCoefficient & allcoeffs){
    CHECK(json.is_array());
    for(auto && dat : json){
        AbilityCoefficients c;
        from_json(dat, c);
        allcoeffs.push_back(c);
    }
}
void from_json(const nlohmann::json & json, AbilityInfo & info){
    if(json.contains("coefficients")){
        from_json(json["coefficients"], info.coefficients);
    }
    if(json.contains("Number of Ticks")){
        info.nTicks=json["Number of Ticks"].get<int>();
    }
    if(json.contains("Time")){
        info.time=Second(json["Time"].get<double>());
    }
    if(json.contains("Initial Tick")){
        info.extraInitialTick=json["Initial Tick"].get<bool>();
    }
}
}
namespace Simulator::detail {
AbilityInfo getAbilityFromJson(const nlohmann::json & json){
    return json.get<AbilityInfo>();
}
AbilityInfo getDefaultAbilityInfo(AbilityId id) {
    switch (id) {
        case dirty_fighting_dirty_blast:{
            AbilityCoefficients coeffsMH{1.12, 0.112, 0.112, -0.25};
            AbilityCoefficients coeffOH{0.0, 0.0, 0.0, -0.25};
            coeffOH.isOffhandHit = true;
            AbilityCoefficients coeffInternalHit{0.5, 0.05, 0.05, 0.0, DamageType::Internal, false};
            AbilityInfo info{{coeffsMH, coeffOH, coeffInternalHit},AbilityCastType::Cast};
            return info;
        }
        case dirty_fighting_shrap_bomb:{
            return AbilityInfo{AllAbilityCoefficient{{0.27,0.027,0.027,0.0,DamageType::Internal,true,false}},AbilityCastType::Instant,Second(3),8};
        }
        case dirty_fighting_exploited_weakness:{
            return AbilityInfo{AllAbilityCoefficient{{0.112, 0.0112, 0.0112, 0, DamageType::Internal, true,false}},AbilityCastType::Instant,Second(3),6};
        }
        case dirty_fighting_wounding_shots:{
            AbilityCoefficients coeffsMH{0.49, 0.049, 0.049, -0.67};
            AbilityCoefficients coeffOH{0.0, 0.0, 0.0, -0.67};
            coeffOH.isOffhandHit = true;
            return AbilityInfo{{coeffsMH, coeffOH},AbilityCastType::Channeled,Second(1.0),4};
        }
        case dirty_fighting_hemorraghing_blast:{
            AbilityCoefficients coeffsMH{0.17, 0.017, 0.017, -0.89};
            AbilityCoefficients coeffsOH{0.0, 0.0, 0.0, -0.89};
            coeffsOH.isOffhandHit = true;
            return AbilityInfo{{coeffsMH, coeffsOH}};
        }
        case gunslinger_vital_shot:{
            return AbilityInfo{AllAbilityCoefficient{{0.3075, 0.03075, 0.03075, 0, DamageType::Internal, true,false}},AbilityCastType::Instant,Second(3),6};
        }
        case gunslinger_illegal_mods:
        case gunslinger_hunker_down:
        case gunslinger_smugglers_luck:{
            AbilityInfo info{{},AbilityCastType::OffGCD};
        }
        case gunslinger_quickdraw:{
            AbilityCoefficients coeffMH{2.51,0.251,0.251,0.67};
            AbilityCoefficients coeffOH{0.0,0.0,0.0,0.67};
            coeffOH.isOffhandHit = true;
            return AbilityInfo{{coeffMH,coeffOH}};
        }
            
        case smuggler_flurry_of_bolts:{
            AbilityCoefficients coeffMH{1.0,0.0,0.0,0.0};
            AbilityCoefficients coeffOH{0.0,0.0,0.0,0.0};
            coeffOH.isOffhandHit=true;
            return AbilityInfo{{coeffMH,coeffOH}};

        }

            
            
        case tactics_gut:{
            return AbilityInfo{{AbilityCoefficients{0.95, 0.075, 0.115, 0.0, DamageType::Kinetic, false, false}}};
        }
        case tactics_gut_dot:{
            auto ac = AllAbilityCoefficient{{0.25, 0.025, 0.025, 0, DamageType::Internal, true, false}};
            auto ai= AbilityInfo{ac};
            ai.extraInitialTick=true;
            ai.nTicks=6;
            ai.time=Second(3);
            return ai;
        }
        case tactics_tactical_surge:{
            return AbilityInfo{{{1.72, 0.152, 0.192, 0.0, DamageType::Kinetic, false,false}}};
        }
        case tactics_cell_burst:{
            return AbilityInfo{{{0.84, 0.079, 0.089, 0.0, DamageType::Energy}}};
        }
            
            
        case vanguard_stockstrike:{
            return AbilityInfo{{{1.77, 0.158, 0.198, 0.0, DamageType::Kinetic, false,false}}};
        }
            
        case trooper_high_impact_bolt:{
            return AbilityInfo{{{1.97, 0.197, 0.197, 0.31, DamageType::Weapon}}};
        }
            
            
    }
}
} // namespace Simulator::detail

