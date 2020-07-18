#pragma once
#include "Ability.h"
#include "Buff.h"
#include "Target.h"
namespace Simulator {

class RawSheetBuff : public Buff {
  public:
    RawSheetBuff(const std::string &buffName, const AbilityIds &ids, double rawDamageMultipler, double flatCritBonus,
                 double flatCritMultiplier, double ap);
    RawSheetBuff(const std::string &buffName, const AbilityIds &ids, const StatChanges &changes):_ids(ids),_statChanges(changes),_name(buffName){}
    void apply(const Ability &ability, StatChanges &fstats, const Target &target) const final;
    virtual ~RawSheetBuff() = default;
  private:
    AbilityIds _ids;
    StatChanges _statChanges;
    std::string _name;
};

class DamageTypeBuff : public Buff {
  public:
    DamageTypeBuff(const std::string &buffName, const std::vector<DamageType> &damageTypes, double rawDamageMultipler,
                   double flatCritBonus, double flatCritMultiplier, double ap)
        :  _types(damageTypes), _rawMultiplier(rawDamageMultipler), _flatCritBonus(flatCritBonus),
          _flatCritMultiplier(flatCritMultiplier), _armorPen(ap), _name(buffName) {}
    void apply(const Ability &ability, StatChanges &fstats, const Target &target) const final;

  private:
    std::vector<DamageType> _types;
    double _rawMultiplier;
    double _flatCritBonus;
    double _flatCritMultiplier;
    double _armorPen;
    std::string _name;
};


} // namespace Simulator
