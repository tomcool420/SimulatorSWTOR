#include "log.h"
#include "../utility.h"
#include <mutex>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace Simulator::detail {
namespace {
std::mutex mutex;
std::shared_ptr<spdlog::logger> logger;
} // namespace

void redirectLogging(const Sinks &sinks) {
    auto l = std::make_shared<spdlog::logger>("v3d", sinks.begin(), sinks.end());
    l->set_level(spdlog::level::trace);
    l->set_pattern("[%H:%M:%S.%e %t %L] %v");
    l->set_level(spdlog::level::debug);
    l->flush_on(spdlog::level::info);
    std::swap(l, logger);
}
void setupLogging(const Sinks &sinks) {
    CHECK(!logger)
    redirectLogging(sinks);
}
const std::shared_ptr<spdlog::logger> &getLogger() {
    std::lock_guard l(mutex);
    if (!logger) {
        setupLogging({// #if defined(_MSC_VER)
                      //             std::make_shared<spdlog::sinks::msvc_sink_mt>(),
                      // #endif
                      std::make_shared<spdlog::sinks::stdout_sink_mt>()});
    }
    return logger;
}
} // namespace Simulator::detail
