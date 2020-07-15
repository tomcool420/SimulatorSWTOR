#pragma once
#include "Buff.h"
#include "detail/units.h"
#include "types.h"

namespace Simulator {

class StatBuff : public Buff {

  public:
    StatBuff(StatChanges changes) : Buff(), _statChanges(std::move(changes)) {}
    void apply(StatChanges &changes) { changes += _statChanges; }

  private:
    StatChanges _statChanges;
};
} // namespace Simulator
