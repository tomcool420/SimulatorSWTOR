#include "Class.h"
#include "AbilityBuff.h"
#include <Simulator/SimulatorBase/utility.h>
namespace Simulator {
void Class::loadOptions(const nlohmann::json &j) {
    load_to_if(j, key_relic_1, _Relic1);
    load_to_if(j, key_relic_2, _Relic2);
    load_to_if(j, key_class_buffs, _ClassBuffs);
}
nlohmann::json Class::serialize() {
    nlohmann::json j;
    j[key_relic_1] = _Relic1;
    j[key_relic_2] = _Relic2;
    j[key_class_buffs] = _ClassBuffs;
    return j;
}

std::vector<BuffPtr> Class::getStaticBuffs() {
    auto getRelicBuff = [](const AbilityId &id) -> BuffPtr {
        if (id == relic_mastery_surge) {
            return std::make_unique<RelicProcBuff>(relic_mastery_surge, Mastery{2892}, Power{0}, CriticalRating{0.0});
        } else if (id == relic_power_surge) {
            return std::make_unique<RelicProcBuff>(relic_power_surge, Mastery{0.0}, Power{2892}, CriticalRating{0.0});
        } else if (id == relic_critical_surge) {
            return std::make_unique<RelicProcBuff>(relic_power_surge, Mastery{0.0}, Power{2892}, CriticalRating{0.0});
        }
        SIM_INFO("No relic with id {} is supported", id);
        return nullptr;
    };
    BuffPtrs ret;

    if (auto r1 = getRelicBuff(_Relic1))
        ret.push_back(std::move(r1));
    if (auto r2 = getRelicBuff(_Relic2))
        ret.push_back(std::move(r2));

    return ret;
}
} // namespace Simulator