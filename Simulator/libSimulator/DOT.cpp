#include "DOT.h"
#include "Target.h"

namespace Simulator{
DamageHits DOT::tick(const Target &t, const FinalStats &s, const Second &time){
    _lastTickTime=time;
    ++_tickCount;
    return calculateDamageRange(*this, s);
    
}
}
