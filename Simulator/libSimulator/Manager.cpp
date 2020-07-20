#include "Manager.h"
#include "utility.h"
#include "Target.h"

namespace Simulator{
void Manager::addTarget(const TargetPtr &target){
    CHECK(target)
    _targets.insert_or_assign(target->getId(), target);
}
namespace {
struct NextEvent{
    TargetId id;
    Second time;
};
}
void Manager::runEventsUntilTime(const Second & time){
    auto getNextEvent = [&]()->std::optional<NextEvent>{
        std::vector<NextEvent> ne;
        for(auto && [tid,target]: _targets){
            if(auto one = target->getNextEventTime()){
                ne.push_back({tid,*one});
            }
        }
        if(ne.empty())
            return std::nullopt;
        
        std::sort(ne.begin(), ne.end(), [](const NextEvent &a, const NextEvent &b){
            return a.time<b.time;
        });
        
        return ne.front();
    };
    auto nextEvent = getNextEvent();
    while(nextEvent && nextEvent->time<time){
            auto && t = _targets[nextEvent->id];
            t->applyEventsAtTime(time);
        nextEvent=getNextEvent();
    }
    
}



}
