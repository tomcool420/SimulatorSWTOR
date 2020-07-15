#pragma once
#include "types.h"
class Player {
  public:
    Player(WeaponDamage weaponDamage, WeaponDamage offhandDamage, double mastery, double power, double ftpower,
           double critical, double alacrity, double accuracy);

    [[nodiscard]] double getCriticalChance() const;
    [[nodiscard]] double getCriticalMultiplier() const;
    [[nodiscard]] double getAlacrity() const;
    [[nodiscard]] double getFTBonusDamager() const;
    [[nodiscard]] double getWeapBonusDamage() const;
    [[nodiscard]] double getPriWeaponDamage() const;
    [[nodiscard]] double getSecWeaponDamage() const;
}