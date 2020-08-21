#pragma once
#include <cstdint>

namespace Simulator {
constexpr double StandardHealth{11745};
constexpr double BossArmor{14535};

// cast abilities

constexpr uint64_t vanguard_stockstrike{801367882989568};
constexpr uint64_t trooper_high_impact_bolt{2021194429628416};
constexpr uint64_t tactics_tactical_surge{3393260387041280};
constexpr uint64_t tactics_cell_burst{3393354876321792};
constexpr uint64_t tactics_assault_plastique{3393277566910464};
constexpr uint64_t tactics_gut{2029878853500928};

constexpr uint64_t dirty_fighting_hemorraghing_blast{3404620575539200};
constexpr uint64_t dirty_fighting_shrap_bomb{807698664783872};
constexpr uint64_t gunslinger_vital_shot{2115340112756736};
constexpr uint64_t gunslinger_vital_shot_weakened{2115340112756737}; // Not the actual id, just a different debuff
constexpr uint64_t dirty_fighting_dirty_blast{3404611985604608};
constexpr uint64_t dirty_fighting_wounding_shots{807711549685760};
constexpr uint64_t dirty_fighting_exploited_weakness{4308285989585170};
constexpr uint64_t dirty_fighting_bloody_mayhem{4084775891501056};
constexpr uint64_t gunslinger_quickdraw{807243398250496};
constexpr uint64_t gunslinger_smugglers_luck{962811408678912};
constexpr uint64_t gunslinger_entrenched_offsense{4307238017564672};
constexpr uint64_t gunslinger_hunker_down{2044808159821824};
constexpr uint64_t gunslinger_illegal_mods{760484089298944};
constexpr uint64_t gunslinger_xs_freighter_flyby{2524220999335936};
constexpr uint64_t gunslinger_speed_shot{807264873086976};
constexpr uint64_t smuggler_flurry_of_bolts{947955116802048};
constexpr uint64_t gunslinger_snap_shot{3404302747959296};
constexpr uint64_t gunslinger_charged_burst{807157498904576};
constexpr uint64_t gunslinger_take_cover{807024354918400};

constexpr uint64_t consular_double_strike{812728071487488};
constexpr uint64_t consular_saber_strike{947486965366784};
constexpr uint64_t shadow_shadow_strike{812839740637184};
constexpr uint64_t shadow_spinning_strike{2295904832847872};
constexpr uint64_t shadow_whirling_blow{979716399955968};

constexpr uint64_t infiltration_vaulting_slash{3915738863632384};
constexpr uint64_t infiltration_clairvoyant_strike{980042817470464};
constexpr uint64_t infiltration_force_breach{4056665330548736};
constexpr uint64_t infiltration_psychokinetic_blast{3401425119870976};
constexpr uint64_t infiltration_psychokinetic_blast_upheaval{3401425119871281};
constexpr uint64_t infiltration_shadow_technique{979819479171072};

constexpr uint64_t shadow_force_synergy{3496713264300032};
constexpr uint64_t infiltration_infiltration_tactics{813037309132800};
constexpr uint64_t infiltration_clairvoyance{812947114819584};
constexpr uint64_t infiltration_deep_impact{1776553092448256};
constexpr uint64_t infiltration_judgement{3218554002341888};
constexpr uint64_t infiltration_whirling_edge{3218528232538112};
// dots
constexpr uint64_t tactics_gut_dot{2029878853501184};

constexpr uint64_t debuff_assailable{807698664784260};
constexpr uint64_t debuff_marked{2115340112757234};
constexpr uint64_t debuff_shattered{3322127138684928};

constexpr uint64_t tactics_high_energy_cell{3963924101726208};

constexpr uint64_t relic_mastery_surge{4293365273198592};
constexpr uint64_t relic_power_surge{42929443664035840};
constexpr uint64_t relic_critical_surge{4289220629757952};
constexpr uint64_t supercharged_celerity{3413304999411712};
constexpr uint64_t test_buff{9999999999999999};
constexpr uint64_t test_debuff{9999999999999998};

// knight
constexpr uint64_t force_clarity{4328931897376768};
// sentinel stuff
constexpr uint64_t centering{2528580391141376};

// keys
constexpr char key_relic_1[] = "relic1";
constexpr char key_relic_2[] = "relic2";
constexpr char key_class_buffs[] = "class_buffs";
constexpr char key_base_stats[] = "key_base_stats";
constexpr char key_mastery[] = "mastery";
constexpr char key_alacrity_rating[] = "alacrity_rating";
constexpr char key_critical_rating[] = "critical_rating";
constexpr char key_power[] = "power";
constexpr char key_tech_power[] = "tech_power";
constexpr char key_force_power[] = "force_power";
constexpr char key_mainhand_damage[] = "mainhand_damage";
constexpr char key_offhand_damage[] = "offhand_damage";
constexpr char key_armor[] = "armor";
constexpr char key_health[] = "health";
constexpr char key_has_offhand[] = "has_offhand";

constexpr char key_amplifiers[] = "amplifiers";
constexpr char key_periodic_damage[] = "periodic_intensity";
constexpr char key_tech_wizardry[] = "tech_wizardry";
constexpr char key_force_sensitivity[] = "force_sensitivity";
constexpr char key_armor_pen[] = "armor_pen";

constexpr char key_class[] = "class";
constexpr char key_class_dirty_fighting[] = "dirty_fighting";

constexpr char key_lay_low[] = "lay_low";
constexpr char key_established_foothold[] = "established_foothold";
constexpr char key_exploited_weakness[] = "exploited_weakness";

constexpr char key_main_rotation[] = "main_rotation";
} // namespace Simulator
