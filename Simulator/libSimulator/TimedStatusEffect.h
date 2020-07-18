#pragma once
namespace Simulator {
class TimedStatusEffect {
  public:
    TimedStatusEffect(Second startTime, Second duration) noexcept
        : _startTime(startTime), _endTime(startTime + duration){};
    TimedStatusEffect() noexcept : _indeterminate(true){};
    [[nodiscard]] Second getEndTime() const { return _endTime; }
    [[nodiscard]] Second getStartTime() const { return _startTime; }
    [[nodiscard]] bool isIndeterminate() const { return _indeterminate; }
    virtual ~TimedStatusEffect() = default;
  private:
    Second _startTime{0.0};
    Second _endTime{0.0};
    bool _indeterminate{false};
};
} // namespace Simulator
