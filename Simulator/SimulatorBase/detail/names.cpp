#include "names.h"

namespace Simulator::detail {
std::string getAbilityName(const AbilityId &id) {
    switch (id) {
    case vanguard_stockstrike:
        return "Stockstrike";
    case trooper_high_impact_bolt:
        return "High Impact Bolt";
    case tactics_tactical_surge:
        return "Tactical Surge";
    case tactics_cell_burst:
        return "Cell Burst";
    case tactics_assault_plastique:
        return "Assault Plastique";
    case tactics_gut:
        return "Gut";
    case tactics_gut_dot:
        return "Gut Dot";
    case dirty_fighting_hemorraghing_blast:
        return "Hemorraghing Blast";
    case dirty_fighting_shrap_bomb:
        return "Shrap Bomb";
    case dirty_fighting_dirty_blast:
        return "Dirty Blast";
    case dirty_fighting_wounding_shots:
        return "Wounding Shots";
    case dirty_fighting_exploited_weakness:
        return "Exploited Weakness (Dirty Blast)";
    case dirty_fighting_bloody_mayhem:
        return "Bloody Mayhem";
    case gunslinger_entrenched_offsense:
        return "Entrenched Offense";
    case gunslinger_hunker_down:
        return "Hunker Down";
    case gunslinger_vital_shot:
        return "Vital Shot";
    case gunslinger_smugglers_luck:
        return "Smuggler's Luck";
    case gunslinger_illegal_mods:
        return "Illegal Mods";
    case debuff_marked:
        return "Marked";
    case debuff_assailable:
        return "Assailable";
    case debuff_shattered:
        return "Shattered";
    case relic_critical_surge:
        return "Critical Surge";
    case relic_mastery_surge:
        return "Mastery Surge";
    case relic_power_surge:
        return "Power Surge";
    case test_buff:
        return "Test Buff";
    case test_debuff:
        return "Test Debuff";
    case consular_double_strike:
        return "Double Strike";
    case consular_saber_strike:
        return "Saber Strike";
    case shadow_shadow_strike:
        return "Shadow Strike";
    case shadow_spinning_strike:
        return "Spinning Strike";
    case shadow_whirling_blow:
        return "Whirling Blow";
    case infiltration_vaulting_slash:
        return "Vaulting Slash";
    case infiltration_clairvoyant_strike:
        return "Clairvoyant Strike";
    case infiltration_force_breach:
        return "Force Breach";
    case infiltration_psychokinetic_blast:
        return "Psychokinetic Blast";
    case infiltration_psychokinetic_blast_upheaval:
        return "Psychokinetic Blast (Upheaval)";
    case infiltration_shadow_technique:
        return "Shadow Technique";
    case infiltration_clairvoyance:
        return "Clairvoyance";
    case infiltration_deep_impact:
        return "Deep Impact";
    case infiltration_judgement:
        return "Judgement";
    case infiltration_whirling_edge:
        return "Whirling Edge";
    case infiltration_infiltration_tactics:
        return "Infiltration Tactics";
    default:
        return "";
    }
}
} // namespace Simulator::detail
