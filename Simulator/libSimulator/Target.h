#pragma once
#include "Buff.h"
#include "Debuff.h"
#include <unordered_map>

namespace Simulator {
class Target {
  public:
  private:
    double _armor;
    double _health;
    std::unordered_map<int, Buff> _buffs;
    std::unordered_map<int, Debuff> _debuffs;
};
} // namespace Simulator