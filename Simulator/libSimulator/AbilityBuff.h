#pragma once
#include "Ability.h"
#include "Buff.h"
#include "Target.h"
namespace Simulator {
class AbilityBuff : public Buff {
public:
    virtual void apply(const Ability &ability, FinalStats &fstats, const Target &target);
};
using AbilityBuffPtr = std::unique_ptr<AbilityBuff>;

class RawSheetBuff : public AbilityBuff {
  public:
    RawSheetBuff(const std::string &buffName, const AbilityIds &ids, double rawDamageMultipler, double flatCritBonus,
                 double flatCritMultiplier, double ap)
        : AbilityBuff(), _ids(ids), _rawMultiplier(rawDamageMultipler), _flatCritBonus(flatCritBonus),
          _flatCritMultiplier(flatCritMultiplier), _armorPen(ap), _name(buffName){};
    void apply(const Ability &ability, FinalStats &fstats, const Target &target) final;

  private:
    AbilityIds _ids;
    double _rawMultiplier;
    double _flatCritBonus;
    double _flatCritMultiplier;
    double _armorPen;
    std::string _name;
};

class DamageTypeBuff: public AbilityBuff{
public:
DamageTypeBuff(const std::string &buffName,const std::vector<DamageType> &damageTypes, double rawDamageMultipler, double flatCritBonus,
                double flatCritMultiplier, double ap): AbilityBuff(), _types(damageTypes), _rawMultiplier(rawDamageMultipler), _flatCritBonus(flatCritBonus),
                _flatCritMultiplier(flatCritMultiplier), _armorPen(ap), _name(buffName){}
    void apply(const Ability &ability, FinalStats &fstats, const Target &target) final;

private:
  std::vector<DamageType> _types;
  double _rawMultiplier;
  double _flatCritBonus;
  double _flatCritMultiplier;
  double _armorPen;
  std::string _name;
};
} // namespace Simulator
