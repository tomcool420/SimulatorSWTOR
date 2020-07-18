#pragma once
#include "Buff.h"
#include "DOT.h"
#include "Debuff.h"
#include "constants.h"
#include <optional>
#include <unordered_map>
#include <map>
#include "Player.h"
namespace Simulator {
class Target {
    enum class EventClass { Dot, Buff, Debuff };
    struct Event {
        Second Time{0.0};
        EventClass eClass;
        AbilityId aId;
        PlayerId pId;
    };

  public:
    Target(double health, double armor = BossArmor) : _armor(armor), _health(health), _maxHealth(health){};
    bool isExecutable() const { return _health / _maxHealth < 0.3; }
    void applyDamage(const Ability &abl, const FinalStats &stats, const Second &sec);
    void applyDamageHit(const DamageHits &hits, const Second & time);
    double getDefenseChance() const { return _defenseChance; };
    double getArmor() const { return _armor * (1 - 0.2 * _isSundered); }
    // future proofing
    double getInternalDR() const { return 0.0; };
    double getElementalDR() const { return 0.0; };
    DOT * addDOT(DOTPtr dot,PlayerPtr player, const FinalStats &s, const Second &time);
    DOT * refreshDOT(const AbilityId &ablId, const PlayerId &pId, const Second &time);
    std::optional<Second> getNextEventTime();
    void applyEventsAtTime(const Second &time);
    void logHits() const;
  private:
    double _armor;
    double _defenseChance{0.1};
    double _health{100};
    double _maxHealth{100};
    bool _isSundered{true};
    std::vector<Event> _upcomingEvents;
    std::map<AbilityId, std::map<PlayerId, DOTPtr>> _dots;
    std::map<AbilityId, BuffPtr> _buffs;
    std::map<AbilityId, DebuffPtr> _debuffs;
    std::vector<std::pair<Second, DamageHits>> _hits;
};
} // namespace Simulator
