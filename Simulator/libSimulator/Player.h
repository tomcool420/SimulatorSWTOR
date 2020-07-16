#pragma once
#include "types.h"
namespace Simulator{
class Buff;

class Player {
  public:
    Player(WeaponDamage weaponDamage, WeaponDamage offhandDamage, double mastery, double power, double ftpower,
           double critical, double alacrity, double accuracy);
    
private:
    std::vector<Buff> _buffs;
};
}
