#pragma once
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>
namespace Simulator {
#define CHECK(condition, ...)                                                                                          \
    {                                                                                                                  \
        if (!static_cast<bool>(condition)) {                                                                           \
            Simulator::checkFailed(#condition, Simulator::getCheckString(__VA_ARGS__));                                \
        }                                                                                                              \
    }

#define SIMULATOR_SET_MACRO(name, type, default)                                                                       \
  public:                                                                                                              \
    void set##name(type arg) { _##name = arg; }                                                                        \
    const type &get##name() const { return _##name; }                                                                  \
                                                                                                                       \
  private:                                                                                                             \
    type _##name{default};

#define SIMULATOR_GET_ONLY_MACRO(name, type, default)                                                                  \
  public:                                                                                                              \
    [[nodiscard]] const type &get##name() const { return _##name; }                                                    \
                                                                                                                       \
  private:                                                                                                             \
    type _##name{default};

inline std::string getCheckString() { return ""; }
template <class... A> std::string getCheckString(A... a) { return fmt::format(a...); }
inline void checkFailed(const std::string &condition, const std::string &description) {
    auto errorString = fmt::format("Check Failed: {}. {}", condition, description);
    throw std::runtime_error(errorString);
}

template <class T> void load_to_if(const nlohmann::json &j, const std::string_view &k, T &value) {
    auto it = j.find(k);
    if (it == j.end())
        return;
    value = it->get<T>();
}
} // namespace Simulator
