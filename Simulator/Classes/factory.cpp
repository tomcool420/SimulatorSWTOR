#include "factory.h"
#include "DirtyFighting.h"
#include <Simulator/SimulatorBase/Class.h>

namespace Simulator {
ClassPtr createClassFromJson(const nlohmann::json &j) {
    auto className = j.at(key_class).get<std::string>();
    ClassPtr c;
    if (className == key_class_dirty_fighting) {
        c = std::make_shared<DirtyFighting>();
        c->loadOptions(j);
    }

    CHECK(c, "class of type {} is not supported yet", className);
    return c;
}
} // namespace Simulator