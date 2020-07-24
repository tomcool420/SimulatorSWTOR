#pragma once
#include "../libSimulator/Class.h"
#include "../libSimulator/utility.h"
namespace Simulator{
class Gunslinger : public Class{
public:
    
    [[nodiscard]] AbilityInfo getAbilityInfo(AbilityId id) override;
    [[nodiscard]] AbilityPtr getAbility(AbilityId id) override;

     SIMULATOR_SET_MACRO(EstablishedFoothold,bool,false);
     SIMULATOR_SET_MACRO(ExploitedWeakness,bool,false);
};
}
