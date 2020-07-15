#pragma once
namespace Simulator {
class TimedStatusEffect {
  public:
    TimedStatusEffect(double startTime, double duration) noexcept
        : _startTime(_startTime), _endTime(_startTime + duration){};
    [[nodiscard]] double getEndTime() const { return _endTime; }
    [[nodiscard]] double getStartTime() const { return _startTime; }

  private:
    double _startTime;
    double _endTime;
};
} // namespace Simulator