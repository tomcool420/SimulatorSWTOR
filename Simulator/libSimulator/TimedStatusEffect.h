#pragma once
namespace Simulator {
class TimedStatusEffect {
  public:
    TimedStatusEffect(double startTime, double duration) noexcept
        : _startTime(_startTime), _endTime(_startTime + duration){};
    TimedStatusEffect() noexcept : _indeterminate(true){};
    [[nodiscard]] double getEndTime() const { return _endTime; }
    [[nodiscard]] double getStartTime() const { return _startTime; }
    [[nodiscard]] bool isIndeterminate() const { return _indeterminate; }

  private:
    double _startTime{0.0};
    double _endTime{0.0};
    bool _indeterminate{false};
};
} // namespace Simulator