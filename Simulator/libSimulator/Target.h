#pragma once
#include "Buff.h"
#include "Debuff.h"
#include <unordered_map>
#include "DOT.h"

namespace Simulator {
class Target {
    enum class EventClass {Dot,Buff,Debuff};
    struct Event{
        Second Time{0.0};
        EventClass eClass;
        AbilityId aId;
    };
  public:
    bool isExecutable() const { return _health / _maxHealth < 0.3; }
    Second getNextEvent() const;
    void applyDamage(const Ability &abl, const FinalStats & stats, const Second &sec);
    double getDefenseChance() const {return _defenseChance;};
    double getArmor() const {return _armor*(1-0.2*_isSundered);}
    // future proofing
    double getInternalDR() const {return 0.0;};
    double getElementalDR() const {return 0.0;};
    
  private:
    double _armor;
    double _defenseChance;
    double _health{100};
    double _maxHealth{100};
    bool _isSundered{true};
    std::vector<Event> upcomingEvents;
    std::unordered_map<int, DOT> _dots;
    std::unordered_map<int, Buff> _buffs;
    std::unordered_map<int, Debuff> _debuffs;
    std::vector<std::pair<Second,DamageHits>> _hits;
};
} // namespace Simulator
