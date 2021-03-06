#include "PriorityList.h"
#include "Simulator/SimulatorBase/types.h"
#include <vector>

namespace Simulator {
class StaticRotation;
using StaticRotationPtr = std::shared_ptr<StaticRotation>;

class StaticRotation : public RotationalPriorityList {
  public:
    [[nodiscard]] RotationalReturn getNextAbility(const TargetPtr &source, const TargetPtr &target,
                                                  const Second &nextInstant, const Second &nextGCD) override;
    void log(std::ostream &s, int indent) const override;
    void addAbility(AbilityId id) { _rotation.push_back(id); }
    void addPriorityList(const RotationalPriorityListPtr &list) { _rotation.push_back(std::move(list)); }
    void addDelay(Second delay) { _rotation.push_back(delay); }
    [[nodiscard]] int getSize() const { return static_cast<int>(_rotation.size()); }
    [[nodiscard]] int getIndex() const { return _index; }
    nlohmann::json serialize() const override;
    static StaticRotationPtr deserialize(const nlohmann::json &j);
    virtual ~StaticRotation() = default;

  private:
    int _index{0};
    std::vector<std::variant<AbilityId, RotationalPriorityListPtr, Second>> _rotation;
};

} // namespace Simulator