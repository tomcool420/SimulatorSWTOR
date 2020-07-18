#include "Target.h"
#include "utility.h"
namespace Simulator {
std::optional<Second> Target::getNextEventTime() {
    std::vector<Event> events;
    std::vector<std::pair<AbilityId,PlayerId>> idsToRemove;
    for (auto &&[aid, dotInstances] : _dots) {
        for(auto &&[pId,dot]:dotInstances){
            if (!dot || dot->isFinished()) {
                if(!dot){
                    std::cout<<"dot is nullptr"<<std::endl;
                }
                idsToRemove.emplace_back(aid,pId);
            }
            auto nextTime = dot->getNextEvent();
            if (nextTime) {
                events.push_back({*nextTime, EventClass::Dot,aid,pId});
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

DOT * Target::addDOT(DOTPtr dot,PlayerPtr player, const FinalStats &s, const Second &time) {
    auto id = dot->id;
    bool anyInstanceOnDotOnTarget{true};
    DOT * dotptr = dot.get();
    //Apply before doing damage so ticks that check for damage are affected but only if the dot isn't already on target
    auto & dotMap = _dots[id];
    dot->setPlayer(player);
    if(dotMap.size()==0){
        anyInstanceOnDotOnTarget=false;
        auto && itp =dotMap.insert_or_assign(player->getId(), std::move(dot));
        dotptr = itp.first->second.get();
    }
    auto hits = dotptr->apply(*this, s, time);
    // Check if the first dot actually hit
    bool foundHit = false;
    for (auto &&hit : hits) {
        if (hit.miss == false)
            foundHit = true;
    }
    if(!anyInstanceOnDotOnTarget && [&](){
        for(auto && hit : hits)
            if(hit.miss == false)
                return false;
        return true;
    }()){
        // the dot did not get applied to the target
        std::cout << fmt::format("Dot with id {} failed to apply",id) << std::endl;
        _dots.erase(id);
        return nullptr;
    }
    if(anyInstanceOnDotOnTarget){
        dotMap.insert_or_assign(player->getId(), std::move(dot));
    }
    
    
    applyDamageHit(hits, time);
    return dotptr;
}

DOT * Target::refreshDOT(const AbilityId &ablId, const PlayerId &pId, const Second &time){
    auto dotMapIt = _dots.find(ablId);
    CHECK(dotMapIt!=_dots.end());
    auto dotIt = dotMapIt->second.find(pId);
    CHECK(dotIt!= dotMapIt->second.end());
    auto dot = dotIt->second.get();
    auto fs = getFinalStats(*dot, *(dot->getPlayer()), *this);
    auto hits = dot->refresh(*this, fs, time);
    std::cout<<fmt::format("Refreshing dot with id {} at time {}",ablId,time.getValue())<<std::endl;
    applyDamageHit(hits, time);
    return dot;
}
void Target::applyDamageHit(const DamageHits &hits, const Second & time){
    _hits.emplace_back(time,hits);
    
}
void Target::logHits() const{
    for(auto && [time, hits] : _hits){
        for(auto && hit : hits){
            std::cout<<fmt::format("Time : {}, abl: {}, damage: {}, crit: {}, miss: {}, offhand: {}",time.getValue(),hit.id,hit.dmg,hit.crit,hit.miss,hit.offhand)<<std::endl;

        }
    }
}

void Target::applyEventsAtTime(const Second &time){
    for(auto && event:_upcomingEvents){
        if(event.Time>time)
            break;
        switch (event.eClass) {
            case Target::EventClass::Dot:
                {
                    auto dmit = _dots.find(event.aId);
                    CHECK(dmit!=_dots.end());
                    auto dit = dmit->second.find(event.pId);
                    CHECK(dit!=dmit->second.end());
                    auto && p = dit->second->getPlayer();
                    auto && rs = p->getRawStats();
                    auto sc = p->getCurrentPlayerStats(*(dit->second), *this);
                    auto fs = getFinalStats(rs, sc);
                    auto hits = dit->second->tick(*this, fs, time);
                    applyDamageHit(hits, time);
                }
                break;
                
            default:
                break;
        }
    }
}

} // namespace Simulator
