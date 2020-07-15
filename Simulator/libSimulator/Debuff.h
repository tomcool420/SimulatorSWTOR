#pragma once
#include "TimedStatusEffect.h"
namespace Simulator {
class Debuff : public TimedStatusEffect {
  private:
    double _tickRate;
};
} // namespace Simulator
