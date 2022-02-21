#pragma once
#include "Buff.h"
#include "DOT.h"
#include "Debuff.h"
#include "Energy.h"
#include "constants.h"
#include "detail/units.h"
#include "types.h"
#include "utility.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <map>
#include <optional>
#include <unordered_map>

namespace Simulator {
using TargetId = boost::uuids::uuid;
class Target : public std::enable_shared_from_this<Target> {
  public:
    enum class EventClass { Dot, Buff, Debuff, Energy, Cooldown };
    struct Event {
        Second Time{0.0};
        EventClass eClass;
        AbilityId aId;
        TargetId pId;
    };
    enum class TargetEventType {
        AddBuff,
        RemoveBuff,
        RefreshBuff,
        AddDebuff,
        RemoveDebuff,
        RefreshDebuff,
        Die,
        Damage,
        SpendEnergy,
        GainEnergy
    };
    struct TargetEvent {
        TargetEventType type;
        Second time;
        std::optional<DamageHits> damage;
        std::optional<AbilityId> id;
        std::optional<double> amount;
    };
    struct CooldownInfo {
        int maxCharges{1};
        int availableCharges{1};
        Second cooldownDuration;
        std::optional<Second> nextCharge;
    };
    using TargetEvents = std::vector<TargetEvent>;

  private:
    Target(RawStats rs) : _rawStats(rs), _health(rs.hp), _tag(boost::uuids::random_generator()()){};

  public:
    static std::shared_ptr<Target> New(const RawStats &rs) { return std::shared_ptr<Target>(new Target(rs)); }
    TargetId getId() const { return _tag; }

    bool isExecutable() const { return _health / _rawStats.hp < 0.3; }
    void applyDamage(const Ability &abl, const FinalStats &stats, const Second &sec);
    void applyDamageHit(const DamageHits &hits, const TargetPtr &target, const Second &time);
    double getDefenseChance() const { return _defenseChance; };
    Armor getArmor() const { return _rawStats.armor; }
    // future proofing
    double getInternalDR() const { return 0.0; };
    double getElementalDR() const { return 0.0; };
    void addDOT(DOTPtr dot, TargetPtr player, const AllFinalStats &s, const Second &time);
    DOT *refreshDOT(const AbilityId &ablId, const TargetId &pId, const Second &time);
    std::optional<Second> getNextEventTime();
    void applyEventsAtTime(const Second &time);
    void logHits() const;
    const std::vector<std::pair<Second, DamageHits>> &getHits() const { return _hits; }
    void clearHits() { _hits.clear(); }
    void addDebuff(DebuffPtr debuff, TargetPtr player, const Second &time);

    using DebuffMap = std::map<AbilityId, std::map<TargetId, DebuffPtr>>;
    DebuffMap &getDebuffs() { return _debuffs; }
    using BuffMap = std::map<AbilityId, BuffPtr>;
    BuffMap &getBuffs() { return _buffs; }

    void setLogEvents(bool log) { _logEvents = log; }
    const RawStats &getRawStats() const { return _rawStats; }
    AllStatChanges getStatChangesFromBuffs(const Ability &abl, const TargetPtr &target) const;
    AllStatChanges getStatChangesFromDebuff(const Ability &abl, const TargetPtr &source) const;
    bool isBleeding() const;

    void addBuff(BuffPtr buff, const Second &time);
    template <class T> T *getBuff(const AbilityId &aid) {
        auto it = _buffs.find(aid);
        if (it == _buffs.end())
            return nullptr;
        return dynamic_cast<T *>(it->second.get());
    }
    template <class T> T *getDebuff(const AbilityId &aid, const TargetId &tid) {
        auto dotMapIt = _debuffs.find(aid);
        if (dotMapIt == _debuffs.end())
            return nullptr;
        auto dotIt = dotMapIt->second.find(tid);
        if (dotIt == dotMapIt->second.end())
            return nullptr;
        Debuff *ptr = dotIt->second.get();
        return dynamic_cast<T *>(ptr);
    }
    void removeDebuff(const AbilityId &aid, const TargetId &pid, const Second &time);
    void removeBuff(const AbilityId &aid, const Second &time);

    HealthPoints getCurrentHealth() const { return _health; }
    HealthPoints getMaxHealth() const { return _rawStats.hp; }

    const TargetEvents &getEvents() const { return _events; }
    std::optional<Second> getDeathTime() const { return _deathTime; }

    void setAbilityCooldown(const AbilityId &id, const Second &time);
    std::optional<Second> getAbilityCooldownEnd(const AbilityId &id) const;
    void finishCooldown(const AbilityId &abl, const Second &time);

    void putAbilityOnCooldown(const AbilityId &info, const Second &nominalCD, const Second &currentTime,
                              int maxCharges = 1);
    void reduceAbilityCooldown(const AbilityId &info, const Second &reductionTime);
    std::optional<Second> getAbilityUsableCharge();
    std::optional<Second> getAbilityCooldownNextCharge(const AbilityId &id);

    void setAbilityCooldown(const AbilityInfo &info, const Second &duration);
    std::optional<Second> getAbilityCooldownEnd(const AbilityInfo &id) const;
    void finishCooldown(const AbilityInfo &abl, const Second &time);

    void setEnergyModel(EnergyPtr e);
    void addEnergy(int e, const Second &time);
    void spendEnergy(int e, const Second &time);
    const std::shared_ptr<Energy> &getEnergyModel() const { return _energy; }

    void setCurrentHealth(HealthPoints hp) { _health = hp; }

  protected:
    void addEvent(TargetEvent &&event);

  private:
    RawStats _rawStats;
    double _defenseChance{0.1};
    HealthPoints _health{100};
    std::vector<Event> _upcomingEvents;
    BuffMap _buffs;
    DebuffMap _debuffs;
    std::vector<std::pair<Second, DamageHits>> _hits;
    TargetEvents _events;
    TargetId _tag;
    std::optional<Second> _deathTime;
    std::map<AbilityId, CooldownInfo> _abilityCooldowns;
    std::map<AbilityId, Second> _abilityCooldownEnd;
    EnergyPtr _energy;
    bool _logEvents{true};
};

template <class T> void addBuffs(const TargetPtr &t, T v, const Second &time) {
    for (auto &&buff : v) {
        t->addBuff(std::move(buff), time);
    }
}
} // namespace Simulator
