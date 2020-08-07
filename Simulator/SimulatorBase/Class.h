#pragma once
#include "Ability.h"
#include "Buff.h"
#include "detail/Cache.h"
#include "detail/units.h"
#include "types.h"
namespace Simulator {

class Class {
  public:
    [[nodiscard]] const AbilityPtr &getAbility(AbilityId id) {
        return _cache.getFromCacheIfNotIn(id, [&]() { return getAbilityInternal(id); });
    }
    virtual std::vector<BuffPtr> getStaticBuffs();
    virtual EnergyPtr getEnergyModel() { return nullptr; }
    virtual void loadOptions(const nlohmann::json &j);
    virtual nlohmann::json serialize() = 0;
    void onAbilityWasCast(const Ability &abl);
    virtual ~Class() = default;

    SIMULATOR_SET_MACRO(Relic1, AbilityId, 0);
    SIMULATOR_SET_MACRO(Relic2, AbilityId, 0);
    SIMULATOR_SET_MACRO(ClassBuffs, bool, false);

  protected:
    [[nodiscard]] virtual AbilityPtr getAbilityInternal(AbilityId id) = 0;
    [[nodiscard]] auto &getCache() { return _cache; }

  private:
    detail::Cache<AbilityPtr> _cache;
};
} // namespace Simulator
