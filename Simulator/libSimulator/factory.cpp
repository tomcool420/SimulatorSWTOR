#include "factory.h"
#include <Simulator/Classes/factory.h>

namespace Simulator {
ClassPtr getClassFromGearJSON(const nlohmann::json &j) {
    nlohmann::json cj = j.at(key_class);
    return createClassFromJson(cj);
}
RawStats getRawStatsFromGearJSON(const nlohmann::json &j) { return j.at(key_base_stats).get<RawStats>(); }

} // namespace Simulator