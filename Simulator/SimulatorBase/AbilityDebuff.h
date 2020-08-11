#pragma once
#include "Debuff.h"
#include "types.h"
#include <optional>
namespace Simulator {
class RawSheetDebuff : public Debuff {
  public:
    RawSheetDebuff(const std::string &buffName, const AbilityId id, const AbilityIds &ids, const StatChanges &sc)
        : Debuff(id), _ids(ids), _statChanges{sc}, _name(buffName){};
    RawSheetDebuff(const std::string &buffName, const AbilityId id, const DamageTypes &dts, const StatChanges &sc)
        : Debuff(id), _types(dts), _statChanges(sc), _name(buffName) {}
    RawSheetDebuff(const std::string &buffName, const AbilityId id, const StatChanges &sc)
        : Debuff(id), _statChanges(sc), _name(buffName) {}
    void modifyStats(const Ability & /*ability*/, AllStatChanges & /*fstats*/,
                     const std::shared_ptr<const Target> & /*target*/) const final;

    virtual ~RawSheetDebuff() = default;
    Debuff *clone() const override { return new RawSheetDebuff(*this); }

  private:
    std::optional<DamageTypes> _types{std::nullopt};
    std::optional<AbilityIds> _ids{std::nullopt};
    StatChanges _statChanges;
    std::string _name;
};

class ShatteredDebuff : public Debuff {
  public:
    ShatteredDebuff() : Debuff(debuff_shattered){};
    [[nodiscard]] DamageHits onAbilityHit(DamageHits &hits, const Second & /*time*/, const TargetPtr & /*player*/,
                                          const TargetPtr & /*target*/) final;
    void modifyStats(const Ability &ability, AllStatChanges &fstats,
                     const std::shared_ptr<const Target> &target) const final;
    Debuff *clone() const override { return new ShatteredDebuff(*this); }
};
} // namespace Simulator
