#pragma once
#include <spdlog/fmt/fmt.h>
#include <iostream>
namespace Simulator {
#define CHECK(condition, ...)                                                                                          \
    {                                                                                                                  \
        if (!static_cast<bool>(condition)) {                                                                           \
            Simulator::checkFailed(#condition, Simulator::getCheckString(__VA_ARGS__));                                \
        }                                                                                                              \
    }

#define SIMULATOR_SET_MACRO(name,type,default)\
public:\
void set##name(type arg){_##name = arg;}\
const  type & get##name() const {return _##name;}\
private:\
type _##name{default};

inline std::string getCheckString() { return ""; }
template <class... A> std::string getCheckString(A... a) { return fmt::format(a...); }
inline void checkFailed(const std::string &condition, const std::string &description) {
    auto errorString = fmt::format("Check Failed: {}. {}", condition, description);
    throw std::runtime_error(errorString);
}
} // namespace Simulator
