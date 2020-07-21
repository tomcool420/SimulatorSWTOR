#include "AbilityDebuff.h"
#include "Ability.h"

namespace Simulator {
void RawSheetDebuff::modifyStats(const Ability &ability, StatChanges &fstats,
                            const std::shared_ptr<const Target> &) const {
    if(_types.has_value()){
        if (_types->empty() || std::find(_types->begin(), _types->end(), ability.getCoefficients().damageType) != _types->end()) {
            fstats+=_statChanges;
        }
    }else if(_ids.has_value()){
        if (_ids->empty() || std::find(_ids->begin(), _ids->end(), ability.getId()) != _ids->end()) {
            fstats+=_statChanges;
        }
    }else{
        fstats+=_statChanges;
    }
}
} // namespace Simulator
