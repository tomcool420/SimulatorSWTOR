#pragma once
#include "Buff.h"
#include "DOT.h"
#include "Debuff.h"
#include "constants.h"
#include <map>
#include <optional>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "types.h"
#include "detail/units.h"
#include "utility.h"

namespace Simulator {
using TargetId = boost::uuids::uuid;

class Target {
    enum class EventClass { Dot, Buff, Debuff };
    struct Event {
        Second Time{0.0};
        EventClass eClass;
        AbilityId aId;
        TargetId pId;
    };

  public:
    Target(RawStats rs)
    : _rawStats(rs), _health(rs.hp){};
    TargetId getId() const { return _tag; }

    bool isExecutable() const { return _health / _rawStats.hp < 0.3; }
    void applyDamage(const Ability &abl, const FinalStats &stats, const Second &sec);
    void applyDamageHit(const DamageHits &hits, Target &target, const Second &time);
    double getDefenseChance() const { return _defenseChance; };
    Armor getArmor() const { return _rawStats.armor * (1 - 0.2 * _sundered); }
    // future proofing
    double getInternalDR() const { return 0.0; };
    double getElementalDR() const { return 0.0; };
    void addDOT(DOTPtr dot, TargetPtr player, const FinalStats &s, const Second &time);
    DOT *refreshDOT(const AbilityId &ablId, const TargetId &pId, const Second &time);
    std::optional<Second> getNextEventTime();
    void applyEventsAtTime(const Second &time);
    void logHits() const;
    void addDebuff(DebuffPtr /*debuff*/, TargetPtr /*player*/, const Second &/*time*/) {}
    
    using DebuffMap = std::map<AbilityId, std::map<TargetId, DebuffPtr>>;
    DebuffMap &getDebuffs() { return _debuffs; }
    using BuffMap = std::map<AbilityId, BuffPtr>;
    BuffMap & getBuffs() {return _buffs;}

    const RawStats & getRawStats() const {return _rawStats;}
    StatChanges getStatChangesFromBuffs(const Ability &abl, const Target &target) const;
    StatChanges getStatChangesFromDebuff(const Ability &abl, const Target & source) const;
    bool isBleeding() const;

    void addBuff(BuffPtr buff){
        _buffs.insert_or_assign(buff->getId(), std::move(buff));
    }
    
    template <class T>
    T * getDebuff(const AbilityId &aid, const TargetId & tid){
        auto dotMapIt = _debuffs.find(aid);
        CHECK(dotMapIt != _debuffs.end());
        auto dotIt = dotMapIt->second.find(tid);
        CHECK(dotIt != dotMapIt->second.end());
        Debuff * ptr = dotIt->second.get();
        return dynamic_cast<T *>(ptr);
    }
    void removeDebuff(const AbilityId &aid, const TargetId &pid);
  private:
    RawStats _rawStats;
    double _defenseChance{0.1};
    HealthPoints _health{100};
    std::vector<Event> _upcomingEvents;
    BuffMap _buffs;
    DebuffMap _debuffs;
    std::vector<std::pair<Second, DamageHits>> _hits;
    TargetId _tag;

    // this needs to be replaced by a generic debuff
    bool _sundered = true;

};

template<class T>
void addBuffs(Target & t, T v){
    for(auto && buff : v){
        t.addBuff(std::move(buff));
    }
}
} // namespace Simulator
