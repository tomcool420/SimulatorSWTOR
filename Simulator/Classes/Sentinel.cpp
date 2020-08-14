#include "Sentinel.h"
#include <Simulator/SimulatorBase/Buff.h>
namespace Simulator {

std::vector<BuffPtr> Sentinel::getStaticBuffs() {
    auto buffs = ClassBase::getStaticBuffs();
    auto bc = std::make_unique<Buff>();
    bc->setId(centering);
    buffs.push_back(std::move(bc));
    return buffs;
}

} // namespace Simulator