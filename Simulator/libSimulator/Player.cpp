#include "Player.h"
#include "Buff.h"
#include "Ability.h"
namespace Simulator{
StatChanges Player::getCurrentPlayerStats( const Ability &abl, const Target &target) const
{
    StatChanges sc;
    for(auto && buff : _buffs){
        buff->apply(abl,sc,target);
    }
    return sc;
}

}
