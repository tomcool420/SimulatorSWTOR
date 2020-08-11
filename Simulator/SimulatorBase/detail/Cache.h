#pragma once
#include "log.h"
#include "../Ability.h"
#include "../types.h"
#include "../utility.h"
#include <map>
#include <mutex>

namespace Simulator::detail {
template <class T> class Cache {
  public:
    template <class F> const T &getFromCacheIfNotIn(AbilityId key, F f) {
        std::lock_guard lock(_lock);
        auto it = _instanceMapperMap.find(key);
        if (it == _instanceMapperMap.end()) {
            T obj = f();
            SIM_INFO("CACHE: adding object to cache with key: {}", key);
            it = _instanceMapperMap.emplace(key, std::pair{0, obj}).first;
        }
        it->second.first += 1;
        return it->second.second;
    }
    bool releaseFromCache(AbilityId key) {
        std::lock_guard lock(_lock);
        auto it = _instanceMapperMap.find(key);
        if (it->second.first <= 1) {
            SIM_INFO("CACHE: Releasing object");
            _instanceMapperMap.erase(it);
            return true;
        } else {
            SIM_INFO("CACHE: Counter is now at {}", it->second.first);
            it->second.first -= 1;
            return false;
        }
    }

  private:
    std::mutex _lock;
    std::map<AbilityId, std::pair<int, T>> _instanceMapperMap;
};
} // namespace Simulator::detail
