#pragma once
#include "Buff.h"
#include "detail/units.h"
#include "types.h"

namespace Simulator {

class StatBuff : public Buff {

  public:
    StatBuff(StatChanges changes) : Buff(), _statChanges(std::move(changes)) {}
    void apply(const Ability &, AllStatChanges &fstats, const TargetPtr &) const final {
        for (auto &&stats : fstats) {
            stats += _statChanges;
        }
    }
    [[nodiscard]] Buff *clone() const override { return new StatBuff(*this); }

  private:
    StatChanges _statChanges;
};

class AbilityStatBuff : public Buff {
  public:
    AbilityStatBuff(StatChanges changes, AbilityIds ids)
        : Buff(), _statChanges(std::move(changes)), _ids(std::move(ids)) {}
    void apply(const Ability &abl, AllStatChanges &fstats, const TargetPtr &) const final {
        if (std::find(_ids.begin(), _ids.end(), abl.getId()) == _ids.end())
            return;
        for (auto &&stats : fstats) {
            stats += _statChanges;
        }
    }
    [[nodiscard]] Buff *clone() const override { return new AbilityStatBuff(*this); }

  private:
    StatChanges _statChanges;
    AbilityIds _ids;
};
} // namespace Simulator
