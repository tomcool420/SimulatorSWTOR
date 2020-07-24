#pragma once
#include "Gunslinger.h"

namespace Simulator{
class DirtyFighting: public Gunslinger{
    
protected:
    AbilityPtr getAbilityInternal(AbilityId id) override;
};
}
