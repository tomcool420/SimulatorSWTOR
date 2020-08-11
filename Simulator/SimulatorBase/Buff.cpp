#include "Buff.h"
#include <mutex>

namespace Simulator {
namespace {
uint64_t _lastIndex = 1000;
std::mutex _mutex;
} // namespace
uint64_t getNextFreeId() {
    std::lock_guard m(_mutex);
    return _lastIndex++;
}

} // namespace Simulator
