#pragma once
#include "TimedStatusEffect.h"
#include <optional>
#include "types.h"
#include "utility.h"
namespace Simulator {
enum class DebuffEventType{
    Apply,
    Remove,
    Tick
};
struct DebuffEvent{
    DebuffEventType type;
    Second time;
    DamageHits hits;
};
using DebuffEvents = std::vector<DebuffEvent>;

class Debuff : public TimedStatusEffect {
  public:
    [[nodiscard]] virtual DamageHits onAbilityHit(DamageHits & /*hits*/, const Second & /*time*/, Target & /*player*/,
                                                  Target & /*target*/) {return {};}
    [[nodiscard]] virtual DebuffEvents resolveEventsUpToTime(const Second &time, Target &t)=0;
    [[nodiscard]] virtual Debuff *clone() const = 0;
    void setSource(TargetPtr source) {_source =std::move(source);}
    [[nodiscard]] const TargetPtr & getSource() const {return _source;}
    
private:
    TargetPtr _source;

    SIMULATOR_SET_MACRO(Bleeding,bool,false);
    SIMULATOR_SET_MACRO(Burning,bool,false);
    SIMULATOR_SET_MACRO(Poisoned,bool,false);
    SIMULATOR_SET_MACRO(Unique,bool,false);
};
using DebuffPtr = std::unique_ptr<Debuff>;
} // namespace Simulator
