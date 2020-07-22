#include "AbilityDebuff.h"
#include "Ability.h"

namespace Simulator {
void RawSheetDebuff::modifyStats(const Ability &ability, AllStatChanges &fstats,
                                 const std::shared_ptr<const Target> &) const {
    if(ability.getCoefficients().empty())
        return;
    auto && c = ability.getCoefficients();
    CHECK(c.size()==fstats.size());
    if(_types.has_value()){
        for(int ii = 0; ii< c.size();++ii){
            if (_types->empty() || std::find(_types->begin(), _types->end(), c[ii].damageType) != _types->end()) {
                fstats[ii]+=_statChanges;
            }
        }
    }else if(_ids.has_value()){
        if (_ids->empty() || std::find(_ids->begin(), _ids->end(), ability.getId()) != _ids->end()) {
            for(int ii = 0; ii< c.size();++ii){
                fstats[ii]+=_statChanges;
            }
        }
    }else{
        for(int ii = 0; ii< c.size();++ii){
            fstats[ii]+=_statChanges;
        }
    }
}
} // namespace Simulator
