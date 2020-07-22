#include "Rotation.h"
#include "Ability.h"
#include "detail/helpers.h"
#include "detail/log.h"
#include "detail/names.h"

namespace Simulator{
std::optional<Second> SetRotation::getNextEventTime(){
    if(_currentAbility){
        auto && info = _currentAbility->getInfo();
        switch (info.type) {
            case AbilityCastType::Instant:
                CHECK(false, "instant casts should never cause a future rotational event");
                break;
            case AbilityCastType::OffGCD:
                CHECK(false, "Off GCD abilities should never cause a future rotational event");
                break;
            case AbilityCastType::Cast:
                return _nextFreeGCD+info.time;
            case AbilityCastType::Channeled:
                return _nextFreeGCD+_currentTick*info.time;
        }
    }
    if(_idCounter<_ids.size()){
        return _nextFreeGCD;
    }else if(++_iterationCount<_Repeats){
        _idCounter=0;
        return _nextFreeGCD;
    }
    return std::nullopt;
}
enum class TargetType{
    Rotation,
    Target,
    Source,
};
struct NextEvent{
    TargetType type;
    Second time;
    int targetCount{0};
};
void SetRotation::doRotation(){
    CHECK(_target);
    
    auto getEvents = [&](){
        std::vector<NextEvent> events;
        if(auto nextSourceEvent = getSource()->getNextEventTime()){
            events.push_back(NextEvent{TargetType::Source,*nextSourceEvent});
        }
        if(auto nextTargetEvent = _target->getNextEventTime()){
            events.push_back(NextEvent{TargetType::Target,*nextTargetEvent});
        }
        if(auto nextRotationEvent = getNextEventTime()){
            events.push_back(NextEvent{TargetType::Rotation,*nextRotationEvent});
        }
        std::sort(events.begin(),events.end(),[](const NextEvent &a, const NextEvent &b){
            return a.time<b.time;
        });
        return events;
    };
    
    auto ne = getEvents();
    while(ne.size()>0){
        auto && event = ne[0];
        if(event.type==TargetType::Source){
            getSource()->applyEventsAtTime(event.time+Second(1e-7));
        }else if(event.type==TargetType::Target){
            _target->applyEventsAtTime(event.time+Second(1e-7));
        }else if(event.type==TargetType::Rotation){
            resolveEventsUpToTime(event.time+Second(1e-7), _target);
        }
        ne = getEvents();
    }
}

void SetRotation::resolveEventsUpToTime(const Second &time, const TargetPtr & target){
    if(_currentAbility){
        auto && info = _currentAbility->getInfo();
        switch (info.type) {
            case AbilityCastType::Cast:{
                CHECK(_nextFreeGCD+info.time<time+Second(1e-5));
                auto fs = getAllFinalStats(*_currentAbility, getSource(), target);
                auto hits = getHits(*_currentAbility, fs, target);
                applyDamageToTarget(hits, getSource(), target, _nextFreeGCD+info.time);
                _currentAbility->onAbilityHitTarget(hits, getSource(), target, _nextFreeGCD+info.time);
                _currentAbility=nullptr;
                _nextFreeGCD=_nextFreeGCD+info.time+Second(1e-6);
                
            }
                break;
            case AbilityCastType::Channeled:{
                CHECK(_nextFreeGCD+info.time<time+Second(1e-5));

                auto fs = getAllFinalStats(*_currentAbility, getSource(), target);
                auto hits = getHits(*_currentAbility, fs, target);
                applyDamageToTarget(hits, getSource(), target, _nextFreeGCD+_currentTick*info.time);
                _currentAbility->onAbilityHitTarget(hits, getSource(), target, _nextFreeGCD+_currentTick*info.time);
                _currentTick+=1;

                if(_currentTick==info.nTicks){
                    _currentAbility=nullptr;
                    _nextFreeGCD=_nextFreeGCD+(_currentTick-1)*info.time+Second(1e-6)+getDelayAfterChanneled();
                    _currentTick=0;
                    break;
                }

            }
            break;
            default:
                break;
        }
    }else{
        auto abl = getAbility(_ids[_idCounter]);
        auto && info = abl->getInfo();
        SIM_INFO("Time: {}, Casting ability [{} : {}]",time.getValue(),detail::getAbilityName(_ids[_idCounter]),_ids[_idCounter]);
        switch (info.type) {
            case AbilityCastType::Cast:
                _currentAbility=abl;
                break;
            case AbilityCastType::Channeled:{
                _currentAbility=abl;
                auto fs = getAllFinalStats(*_currentAbility, getSource(), target);
                auto hits = getHits(*_currentAbility, fs, target);
                applyDamageToTarget(hits, getSource(), target, _nextFreeGCD);
                _currentAbility->onAbilityHitTarget(hits, getSource(), target, _nextFreeGCD);
                _currentTick=1;
                break;
            }
            case AbilityCastType::Instant:{
                auto fs = getAllFinalStats(*abl, getSource(), target);
                auto hits = getHits(*abl, fs, target);
                applyDamageToTarget(hits, getSource(), target, _nextFreeGCD);
                abl->onAbilityHitTarget(hits, getSource(), target, _nextFreeGCD);
                _nextFreeGCD=_nextFreeGCD+Second(1.5);
                break;
            }
            case AbilityCastType::OffGCD:{
                auto fs = getAllFinalStats(*abl, getSource(), target);
                auto hits = getHits(*abl, fs, target);
                applyDamageToTarget(hits, getSource(), target, _nextFreeGCD);
                abl->onAbilityHitTarget(hits, getSource(), target, _nextFreeGCD);
                _nextFreeGCD=_nextFreeGCD+Second(0.001);
            }
            default:
                break;
        }
        ++_idCounter;
    }
    
}
}
