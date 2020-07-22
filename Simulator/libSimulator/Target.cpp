#include "Target.h"
#include "detail/log.h"
#include "utility.h"
#include "detail/names.h"
namespace Simulator {
std::optional<Second> Target::getNextEventTime() {
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
    CHECK(fs.size()==1);
    dot->apply(fs[0], time);
    auto it = dotMap.find( source->getId());
    if (it != dotMap.end()) {
        it->second->refresh(time);
    } else {
        dotMap.insert_or_assign(source->getId(), std::move(dot));
    }
}

DOT *Target::refreshDOT(const AbilityId &ablId, const TargetId &pId, const Second &time) {
    auto dot = getDebuff<DOT>(ablId, pId);
    dot->refresh(time);
        SIM_INFO("Time : {}, abl: [{} {}], refreshing debuff",time.getValue(),detail::getAbilityName(dot->getId()), dot->getId());
    return dot;
}
void Target::addDebuff(DebuffPtr debuff, TargetPtr player, const Second &time) {
    SIM_INFO("Time : {}, abl: [{} {}], applying debuff",time.getValue(),detail::getAbilityName(debuff->getId()), debuff->getId());
    CHECK(debuff);
    auto id = debuff->getId();
    auto &debuffMap = _debuffs[id];
    if (debuffMap.size() && debuff->getUnique())
        debuffMap.clear();

    auto it = debuffMap.find( player->getId());

    if (it != debuffMap.end()) {
        it->second->refresh(time);
    } else {
        debuff->setSource(player);
        FinalStats fs;
        if(auto dot = dynamic_cast<DOT *>(debuff.get())){
            auto stats = getAllFinalStats(dot->getAbility(), player, shared_from_this());\
            fs = stats[0];
        }
        debuff->apply(fs, time);
        debuffMap.insert_or_assign(player->getId(), std::move(debuff));
    }
}

void Target::applyDamageHit(const DamageHits &hits, const TargetPtr & /*player*/, const Second &time) {
    for (auto &&hit : hits) {
        SIM_INFO("Time : {}, abl: [{} {}], damage: {}, crit: {}, miss: {}, offhand: {}, type: {}", time.getValue(), detail::getAbilityName(hit.id), hit.id,
                 hit.dmg, hit.crit, hit.miss, hit.offhand, static_cast<int>(hit.dt));
    }
    _hits.emplace_back(time, hits);
    int sum = 0;
    for (auto hit : hits) {
        if (hit.miss)
            continue;
        sum += std::round(hit.dmg);
    }
    _health -= HealthPoints(sum);
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
    for (auto &&event : _upcomingEvents) {
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
    SIM_INFO("Time : {}, abl: [{} {}], removing debuff",time.getValue(),detail::getAbilityName(event.aId), event.aId);
                    removeDebuff(event.aId, event.pId);
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
void Target::removeDebuff(const AbilityId &aid, const TargetId &pid) {
    auto dotMapIt = _debuffs.find(aid);
    CHECK(dotMapIt != _debuffs.end());
    auto dotIt = dotMapIt->second.find(pid);
    CHECK(dotIt != dotMapIt->second.end());

    if (dotMapIt->second.size() == 1) {
        _debuffs.erase(dotMapIt);
    } else {
        dotMapIt->second.erase(pid);
    }
}
} // namespace Simulator
