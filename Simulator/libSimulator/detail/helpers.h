#include "../Ability.h"
#include "../AbilityBuff.h"
#include "../StatBuff.h"
#include "../constants.h"
#include "../detail/calculations.h"
namespace Simulator {
AbilityPtr getAbility(AbilityId id);
DebuffPtr getDebuff(AbilityId id);
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

  private:
    int _stacks = 0;
};
} // namespace Simulator
