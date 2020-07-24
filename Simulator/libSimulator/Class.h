#pragma once
#include "Ability.h"
#include "types.h"
#include "detail/units.h"
#include "detail/Cache.h"
#include "Buff.h"

namespace Simulator {

class Class {
    [[nodiscard]] virtual AbilityPtr getAbility(AbilityId id)=0;
    [[nodiscard]] virtual std::optional<Second> getNextEventTime()=0;
    [[nodiscard]] virtual std::vector<BuffPtr> getBuffs()=0;
    
  protected:
    [[nodiscard]] virtual AbilityInfo getAbilityInfo(AbilityId id)=0;
    [[nodiscard]] auto & getCache() {return _cache;}
private:
    detail::Cache<AbilityPtr> _cache;
};
} // namespace Simulator
