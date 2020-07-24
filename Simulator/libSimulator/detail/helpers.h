#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../StatBuff.h"
#include "../constants.h"
#include "../detail/calculations.h"
#include "../detail/names.h"

namespace Simulator {
AbilityPtr getAbility(AbilityId id);
DebuffPtr getDebuff(AbilityId id);
BuffPtr getBuff(AbilityId id);
DOTPtr getDot(AbilityId id);
AbilityPtr createDotAbility(AbilityId id);
std::vector<BuffPtr> getTacticsSheetBuffs();
StatChanges getDefaultStatsBuffs(bool twoPiece = true, bool mastery = true);
RawStats getDefaultStats();
BuffPtr getDefaultStatsBuffPtr(bool twoPiece = true, bool mastery = true);

class EnergyLodeBuff : public Buff {
  public:
    EnergyLodeBuff() : Buff(), _stacks(0) { setId(tactics_high_energy_cell); }
    void onAbilityUsed(const Ability &ability, const Second & /*time*/, const TargetPtr & /*player*/,
                       const TargetPtr & /*target*/) override {
        if (ability.getId() == trooper_high_impact_bolt) {
            setStacks(std::min<int>(4, _stacks));
        }
    }
    DamageHits onAbilityHit(DamageHits &hits, const Second & /*time*/, const TargetPtr & /*player*/,
                            const TargetPtr & /*target*/) override {
        for (auto &&hit : hits) {
            if (hit.id == tactics_cell_burst && hit.miss == false) {
                _stacks = 0;
            }
        }
        return {};
    }

    void apply(const Ability &ability, AllStatChanges &fstats, const TargetPtr & /*target*/) const override {
        if (ability.getId() != tactics_cell_burst)
            return;
        for (auto &&stats : fstats) {
            stats.multiplier += 1.05 * std::max(0, getStacks() - 1);
        }
    }

    void setStacks(int stacks) { _stacks = stacks - 1; }
    int getStacks() const { return std::clamp(_stacks + 1, 0, 4); }
    [[nodiscard]] Buff * clone() const override {return new EnergyLodeBuff(*this);}

  private:
    int _stacks = 0;
};

class ColdBloodedBuff : public Buff {
  public:
    ColdBloodedBuff() : Buff(){ }

    DamageHits onAbilityHit(DamageHits &hits, const Second & /*time*/, const TargetPtr & /*player*/,
                            const TargetPtr & target) override {
        if(target->getCurrentHealth()/target->getMaxHealth()<0.3){
            for (auto &&hit : hits) {
                if (hit.id == dirty_fighting_exploited_weakness ||
                    hit.id == dirty_fighting_shrap_bomb ||
                    hit.id == gunslinger_vital_shot ||
                    (hit.id == dirty_fighting_dirty_blast && hit.dt == DamageType::Internal)) {
                    SIM_INFO("APPLYING COLD BLOODED TO ABILITY {}", detail::getAbilityName(hit.id));
                    hit.dmg*=1.15;
                }
            }
        }
        return {};
    }
    [[nodiscard]] Buff * clone() const override {return new ColdBloodedBuff(*this);}

};

class BloodyMayhemDebuff : public Debuff {
  public:
    BloodyMayhemDebuff() : Debuff(dirty_fighting_bloody_mayhem){
        setDuration(Second(15.0));
    }

    DamageHits onAbilityHit(DamageHits &hits, const Second & time, const TargetPtr & player,
                            const TargetPtr & target) override {
        if(_triggered)
            return {};
        AbilityCoefficients coeffs{0.396,0.0396,0.0396,0.0,DamageType::Internal};
        AbilityInfo info{{coeffs}};
        Ability abl(dirty_fighting_bloody_mayhem,info);
        for (auto &&hit : hits) {
            if (hit.id == dirty_fighting_shrap_bomb) {

                _triggered=true;
                break;
            }
        }
        if(_triggered){
            DamageHits ret;
            auto afs = getAllFinalStats(abl, player, target);
            auto newHits = getHits(abl, afs, target);
            ret.insert(ret.end(), newHits.begin(), newHits.end());
            setDuration(time-getStartTime()+Second(1e-5));
            return ret;
        }
        return {};
        
    }
    [[nodiscard]] Debuff *clone() const override {return new BloodyMayhemDebuff(*this);}
private:
    bool _triggered{false};
};

struct AbilityLogInformation{
    AbilityId id;
    double totalDamage{0};
    int hitCount{0};
    int critCount{0};
    int missCount{0};
};

std::map<AbilityId, AbilityLogInformation> getEventInformation(const TargetPtr & target);
} // namespace Simulator
