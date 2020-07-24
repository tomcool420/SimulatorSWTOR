#pragma once
#include "Ability.h"
#include "Buff.h"
#include "detail/Cache.h"
#include "detail/units.h"
#include "types.h"

namespace Simulator {

class Class {
    [[nodiscard]] const AbilityPtr &getAbility(AbilityId id) {
        return _cache.getFromCacheIfNotIn(id, [&]() { return getAbilityInternal(id); });
    }
    //    [[nodiscard]] virtual std::optional<Second> getNextEventTime()=0;
    //    [[nodiscard]] virtual std::vector<BuffPtr> getBuffs()=0;

    void onAbilityWasCast(const Ability &abl);

  protected:
    [[nodiscard]] virtual AbilityPtr getAbilityInternal(AbilityId id) = 0;
    [[nodiscard]] auto &getCache() { return _cache; }

  private:
    detail::Cache<AbilityPtr> _cache;
};
} // namespace Simulator
