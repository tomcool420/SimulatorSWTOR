#include "Target.h"
#include "detail/log.h"
#include "detail/names.h"
#include "utility.h"
#include <boost/uuid/uuid_io.hpp>
namespace Simulator {
std::optional<Second> Target::getNextEventTime() {
    if (_health <= HealthPoints(0)) {
        _upcomingEvents.clear();
        return std::nullopt;
    }
    std::vector<Event> events;
    for (auto &&[aid, debuffInstances] : _debuffs) {
        for (auto &&[pId, debuff] : debuffInstances) {
            CHECK(debuff);
            auto nextTime = debuff->getNextEventTime();
            if (nextTime) {
                events.push_back({*nextTime, EventClass::Debuff, aid, pId});
            }
        }
    }
    for (auto &&[aid, buff] : _buffs) {
        CHECK(buff);
        if (auto ne = buff->getNextEventTime()) {
            events.push_back({*ne, EventClass::Buff, aid, getId()});
        }
    }

    std::sort(events.begin(), events.end(), [](const Event &a, const Event &b) { return a.Time < b.Time; });
    _upcomingEvents = std::move(events);
    if (_upcomingEvents.size()) {
        return _upcomingEvents[0].Time;
    }
    return std::nullopt;
}

void Target::addDOT(DOTPtr dot, TargetPtr source, const AllFinalStats &fs, const Second &time) {
    CHECK(dot);
    auto id = dot->getId();
    auto &dotMap = _debuffs[id];
    dot->setSource(source);
    CHECK(fs.size() == 1);
    dot->apply(fs[0], time);
    auto it = dotMap.find(source->getId());
    if (it != dotMap.end()) {
        it->second->refresh(time);
    } else {
        dotMap.insert_or_assign(source->getId(), std::move(dot));
    }
}
void Target::addBuff(BuffPtr buff, const Second &time) {
    addEvent({TargetEventType::AddBuff, time, std::nullopt, buff->getId()});
    _buffs.insert_or_assign(buff->getId(), std::move(buff));
}

DOT *Target::refreshDOT(const AbilityId &ablId, const TargetId &pId, const Second &time) {
    auto dot = getDebuff<DOT>(ablId, pId);
    dot->refresh(time);
    addEvent({TargetEventType::RefreshDebuff, time, std::nullopt, dot->getId()});
    return dot;
}
void Target::addDebuff(DebuffPtr debuff, TargetPtr player, const Second &time) {
    CHECK(debuff);
    auto id = debuff->getId();
    auto &&bi = debuff->getBlockedByDebuffs();
    bool blocked = false;
    for (int ii = 0; ii < bi.size(); ++ii) {
        for (auto &&it : _debuffs) {
            if (it.first == bi[ii] && !it.second.empty()) {
                blocked = true;
                SIM_INFO("Debuff [{} {}] application blocked by debuff [{} {}] at time {}",
                         detail::getAbilityName(debuff->getId()), debuff->getId(), detail::getAbilityName(it.first),
                         it.first, time);
                break;
            }
        }
        if (blocked)
            break;
    }
    if (blocked)
        return;

    auto &debuffMap = _debuffs[id];
    if (debuffMap.size() && debuff->getUnique())
        debuffMap.clear();

    auto it = debuffMap.find(player->getId());

    if (it != debuffMap.end()) {
        it->second->refresh(time);
        addEvent({TargetEventType::RefreshDebuff, time, std::nullopt, it->second->getId()});
    } else {
        debuff->setSource(player);
        FinalStats fs;
        if (auto dot = dynamic_cast<DOT *>(debuff.get())) {
            auto stats = getAllFinalStats(dot->getAbility(), player, shared_from_this());
            fs = stats[0];
        }
        debuff->apply(fs, time);
        addEvent({TargetEventType::AddDebuff, time, std::nullopt, debuff->getId()});
        debuffMap.insert_or_assign(player->getId(), std::move(debuff));
    }
}

void Target::applyDamageHit(const DamageHits &hits, const TargetPtr & /*player*/, const Second &time) {
    _hits.emplace_back(time, hits);
    addEvent({TargetEventType::Damage, time, hits, std::nullopt});
    int sum = 0;
    for (auto hit : hits) {
        if (hit.miss)
            continue;
        sum += static_cast<int>(std::round(hit.dmg));
    }
    _health -= HealthPoints(sum);
    if (_health <= HealthPoints(0.0)) {
        addEvent({TargetEventType::Die, time, std::nullopt, std::nullopt});
    }
}

void Target::logHits() const {
    for (auto &&[time, hits] : _hits) {
        for (auto &&hit : hits) {
            std::cout << fmt::format("Time : {}, abl: {}, damage: {}, crit: {}, miss: {}, offhand: {}, type: {}\n",
                                     time.getValue(), hit.id, hit.dmg, hit.crit, hit.miss, hit.offhand,
                                     static_cast<int>(hit.dt));
        }
    }
    std::cout << fmt::format("There are {} total hits", _hits.size()) << std::endl;
}

void Target::applyEventsAtTime(const Second &time) {
    for (int ii = 0; ii < _upcomingEvents.size(); ++ii) {
        auto &&event = _upcomingEvents[ii];
        if (event.Time > time)
            break;
        if (event.eClass == Target::EventClass::Debuff) {
            auto debuff = getDebuff<Debuff>(event.aId, event.pId);
            CHECK(debuff);
            auto events = debuff->resolveEventsUpToTime(time, shared_from_this());
            for (auto &&devent : events) {
                if (devent.type == DebuffEventType::Tick) {
                    applyDamageToTarget(devent.hits, (debuff->getSource()), shared_from_this(), time);
                } else if (devent.type == DebuffEventType::Remove) {
                    removeDebuff(event.aId, event.pId, time);
                }
            }
        } else if (event.eClass == Target::EventClass::Buff) {
            auto buff = getBuff<Buff>(event.aId);
            CHECK(buff);
            auto events = buff->resolveEventsUpToTime(time, shared_from_this());
            for (auto &&devent : events) {
                if (devent.type == DebuffEventType::Tick) {
                    CHECK(false, "Can't have a buff tick damage...");
                } else if (devent.type == DebuffEventType::Remove) {
                    removeBuff(event.aId, time);
                }
            }
        }
        getNextEventTime();
    }
}

AllStatChanges Target::getStatChangesFromBuffs(const Ability &abl, const TargetPtr &target) const {
    AllStatChanges sc(abl.getCoefficients().size());
    for (auto &&[aid, buff] : _buffs) {
        buff->apply(abl, sc, target);
    }
    return sc;
}

AllStatChanges Target::getStatChangesFromDebuff(const Ability &abl, const TargetPtr &) const {
    AllStatChanges sc(abl.getCoefficients().size());
    for (auto &&[aid, debuffMap] : _debuffs) {
        for (auto &&[pid, debuff] : debuffMap) {
            debuff->modifyStats(abl, sc, shared_from_this());
        }
    }
    return sc;
}

bool Target::isBleeding() const {
    for (auto &&d : _debuffs) {
        if (d.second.empty())
            continue;
        if (d.second.begin()->second->getBleeding())
            return true;
    }
    return false;
}
void Target::removeDebuff(const AbilityId &aid, const TargetId &pid, const Second &time) {
    auto dotMapIt = _debuffs.find(aid);
    CHECK(dotMapIt != _debuffs.end());
    auto dotIt = dotMapIt->second.find(pid);
    CHECK(dotIt != dotMapIt->second.end());

    if (dotMapIt->second.size() == 1) {
        _debuffs.erase(dotMapIt);
    } else {
        dotMapIt->second.erase(pid);
    }
    addEvent({TargetEventType::RemoveDebuff, time, std::nullopt, aid});
}
void Target::removeBuff(const AbilityId &aid, const Second &time) {
    auto dotMapIt = _buffs.find(aid);
    CHECK(dotMapIt != _buffs.end());
    _buffs.erase(dotMapIt);
    addEvent({TargetEventType::RemoveBuff, time, std::nullopt, aid});
}

void Target::addEvent(TargetEvent &&event) {
    if (event.type == TargetEventType::Damage) {
        CHECK(event.damage.has_value())
        auto &&hits = event.damage.value();
        for (auto &&hit : hits) {
            SIM_INFO("Time : {}, abl: [{} {}], damage: {}, crit: {}, miss: {}, offhand: {}, type: {}",
                     event.time.getValue(), detail::getAbilityName(hit.id), hit.id, hit.dmg, hit.crit, hit.miss,
                     hit.offhand, static_cast<int>(hit.dt));
        }
    } else if (event.type == TargetEventType::AddBuff) {
        SIM_INFO("Time : {}, abl: [{} {}], Adding Buff", event.time.getValue(),
                 detail::getAbilityName(event.id.value()), event.id.value());
    } else if (event.type == TargetEventType::RemoveBuff) {
        SIM_INFO("Time : {}, abl: [{} {}], Removing Buff", event.time.getValue(),
                 detail::getAbilityName(event.id.value()), event.id.value());
    } else if (event.type == TargetEventType::RefreshBuff) {
        SIM_INFO("Time : {}, abl: [{} {}], Refreshing Buff", event.time.getValue(),
                 detail::getAbilityName(event.id.value()), event.id.value());
    } else if (event.type == TargetEventType::AddDebuff) {
        SIM_INFO("Time : {}, abl: [{} {}], Adding Debuff", event.time.getValue(),
                 detail::getAbilityName(event.id.value()), event.id.value());
    } else if (event.type == TargetEventType::RemoveDebuff) {
        SIM_INFO("Time : {}, abl: [{} {}], Removing Debuff", event.time.getValue(),
                 detail::getAbilityName(event.id.value()), event.id.value());
    } else if (event.type == TargetEventType::RefreshDebuff) {
        SIM_INFO("Time : {}, abl: [{} {}], Refreshing Debuff", event.time.getValue(),
                 detail::getAbilityName(event.id.value()), event.id.value());
    } else if (event.type == TargetEventType::Die) {
        SIM_INFO("Time : {},  Target: {}, Death", event.time.getValue(), getId());
        _deathTime = event.time;
    }
    _events.push_back(std::move(event));
}

void Target::setAbilityCooldown(const AbilityId &id, const Second &time) { _abilityCooldownEnd[id] = time; }

std::optional<Second> Target::getAbilityCooldownEnd(const AbilityId &id) const {
    auto it = _abilityCooldownEnd.find(id);
    if (it == _abilityCooldownEnd.end())
        return std::nullopt;
    return it->second;
}

void Target::finishCooldown(const AbilityId &id, const Second &time) { _abilityCooldownEnd[id] = time; }
} // namespace Simulator
