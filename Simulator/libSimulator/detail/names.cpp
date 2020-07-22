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
        case gunslinger_vital_shot:
            return "Vital Shot";
        case debuff_marked:
            return "Marked";
        case debuff_assailable:
            return "Assailable";
        default:
            return "";
    }
}
} // namespace Simulator
