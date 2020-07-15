#pragma once
#include <cstdint>

namespace Simulator {
using AbilityId = uint64_t;
enum class DamageType { Kinetic = 1, Energy = 2, Internal = 3, Elemental = 4 };
class Ability {
  public:
    Ability(AbilityId id);
    [[nodiscard]] AbilityId getId() const { return _id; }

  private:
    AbilityId _id{0};
    double coefficient{0.0};
    double StandardHealthPercentMax{0.0};
    double StandardHealthPercentMin{0.0};
    double AmountModifierPercent{0.0};
    DamageType _damageType{false};
    bool _isDamageOverTime{false};
    bool _isAreaOfEffect{false};
};
} // namespace Simulator
