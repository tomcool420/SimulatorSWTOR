#pragma once
#include <optional>
namespace Simulator {
class TimedStatusEffect {
  public:
    TimedStatusEffect(Second startTime, Second duration) noexcept : _startTime(startTime), _duration(duration){};
    TimedStatusEffect() noexcept : _indeterminate(true){};
    [[nodiscard]] Second getEndTime() const { return _startTime + _duration; }
    [[nodiscard]] Second getStartTime() const { return _startTime; }
    void setDuration(const Second &duration) {
        _duration = duration;
        _indeterminate = false;
    }
    void setStartTime(Second startTime) { _startTime = startTime; }
    [[nodiscard]] bool isIndeterminate() const { return _indeterminate; }
    [[nodiscard]] virtual std::optional<Second> getNextEventTime() const {
        if (_indeterminate) {
            return std::nullopt;
        }
        return _startTime + _duration;
    };
    virtual ~TimedStatusEffect() = default;

  private:
    Second _startTime{0.0};
    Second _duration{0.0};
    bool _indeterminate{false};
};
} // namespace Simulator
