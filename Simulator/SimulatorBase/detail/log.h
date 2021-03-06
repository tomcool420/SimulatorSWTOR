#pragma once
#include <spdlog/fmt/ostr.h>
#include <spdlog/logger.h>

namespace Simulator::detail {
#define SIM_INFO(...)                                                                                                  \
    {                                                                                                                  \
        if (Simulator::detail::getLogger()->should_log(spdlog::level::info)) {                                         \
            Simulator::detail::getLogger()->info(__VA_ARGS__);                                                         \
        }                                                                                                              \
    }

#define SIM_DEBUG(...)                                                                                                 \
    {                                                                                                                  \
        if (Simulator::detail::getLogger()->should_log(spdlog::level::debug)) {                                        \
            Simulator::detail::getLogger()->debug(__VA_ARGS__);                                                        \
        }                                                                                                              \
    }
using Sinks = std::vector<spdlog::sink_ptr>;

const std::shared_ptr<spdlog::logger> &getLogger();
class LogDisabler {
  public:
    LogDisabler() : _level(getLogger()->level()) { getLogger()->set_level(spdlog::level::off); }
    ~LogDisabler() { getLogger()->set_level(_level); }

  private:
    spdlog::level::level_enum _level;
};
} // namespace Simulator::detail
