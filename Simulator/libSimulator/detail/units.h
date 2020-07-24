#pragma once
#include <boost/operators.hpp>
#include <gsl/gsl>
#include <iostream>

namespace Simulator {
template <class T, class Category> class UnitBase : public boost::multiplicative<T, double> {
  public:
    UnitBase() = default;
    explicit constexpr UnitBase(double val) : _val(val) {}
    explicit constexpr UnitBase(long double val) : _val(gsl::narrow_cast<double>(val)) {}
    explicit constexpr UnitBase(int val) : _val(gsl::narrow<double>(val)) {}
    explicit constexpr UnitBase(std::int64_t val) : _val(gsl::narrow<double>(val)) {}

    explicit constexpr operator double() const noexcept { return _val; }
    explicit constexpr operator int() const noexcept { return gsl::narrow_cast<int>(_val); }
    explicit constexpr operator std::int64_t() const noexcept { return gsl::narrow_cast<std::int64_t>(_val); }
    explicit constexpr operator float() const noexcept { return gsl::narrow_cast<float>(_val); }

    constexpr bool operator<(const T &rhs) const noexcept { return _val < rhs._val; }
    constexpr bool operator<=(const T &rhs) const noexcept { return _val <= rhs._val; }
    constexpr bool operator>(const T &rhs) const noexcept { return _val > rhs._val; }
    constexpr bool operator>=(const T &rhs) const noexcept { return _val >= rhs._val; }
    constexpr bool operator==(const T &rhs) const noexcept { return _val == rhs._val; }
    constexpr bool operator!=(const T &rhs) const noexcept { return !(*this == rhs); }

    constexpr T &operator+=(const T &rhs) noexcept {
        _val += rhs._val;
        return static_cast<T &>(*this);
    }

    template <class U> constexpr T operator+(const UnitBase<U, Category> &rhs) const noexcept {
        auto ret = *this;
        return ret += rhs;
    }

    constexpr T &operator-=(const T &rhs) noexcept {
        _val -= rhs._val;
        return static_cast<T &>(*this);
    }

    template <class U> constexpr T operator-(const UnitBase<U, Category> &rhs) const noexcept {
        auto ret = *this;
        return ret -= rhs;
    }

    constexpr T &operator*=(double rhs) noexcept {
        _val *= rhs;
        return static_cast<T &>(*this);
    }

    constexpr T &operator/=(double rhs) noexcept {
        _val /= rhs;
        return static_cast<T &>(*this);
    }

    constexpr T operator-() const noexcept { return T(-_val); }

    constexpr double operator/(const T &rhs) const noexcept { return _val / rhs._val; }

    friend constexpr T abs(const T &x) noexcept { return T(std::abs(x._val)); }
    friend constexpr T round(const T &x) noexcept { return T(std::round(x._val)); }
    friend constexpr T floor(const T &x) noexcept { return T(std::floor(x._val)); }
    friend constexpr T ceil(const T &x) noexcept { return T(std::ceil(x._val)); }
    friend constexpr T fmod(const T &x, const T &y) { return T(std::fmod(x._val, y._val)); }
    friend std::string to_string(const T &x) { return std::to_string(x._val); }

    friend std::ostream &operator<<(std::ostream &os, const T &rhs) { return os << to_string(rhs); }
    friend std::istream &operator>>(std::istream &is, T &rhs) {
        is >> rhs._val;
        return is;
    }
    [[nodiscard]] constexpr double getValue() const noexcept { return _val; }

  protected:
    double _val = 0;
};

// NOLINTNEXTLINE
#define DEFINE_UNIT(name, cat, s)                                                                                      \
    class name : public UnitBase<name, cat> {                                                                          \
      public:                                                                                                          \
        static constexpr name infinity() { return name(std::numeric_limits<double>::infinity()); }                     \
        using category = cat;                                                                                          \
        static constexpr double scale = (s);                                                                           \
        using UnitBase<name, cat>::UnitBase;                                                                           \
        constexpr name(const UnitBase<name, cat> &other) : UnitBase<name, cat>(other) {}                               \
        template <class U>                                                                                             \
        constexpr name(const UnitBase<U, cat> &other) : UnitBase<name, cat>(other.getValue() * (U::scale / scale)) {   \
            static_assert(U::scale == U::scale && scale == scale);                                                     \
        }                                                                                                              \
    };

struct MasteryCategory {};
DEFINE_UNIT(Mastery, MasteryCategory, 1.0);
struct CriticalRatingCategory {};
DEFINE_UNIT(CriticalRating, CriticalRatingCategory, 1.0);
struct AlacrityRatingCategory {};
DEFINE_UNIT(AlacrityRating, AlacrityRatingCategory, 1.0);
struct PowerCategory {};
DEFINE_UNIT(Power, PowerCategory, 1.0);
struct ForceTechDamageCategory {};
DEFINE_UNIT(FTPower, ForceTechDamageCategory, 1.0);
struct AccuracyRatingCategory {};
DEFINE_UNIT(AccuracyRating, AccuracyRatingCategory, 1.0);
struct TimeCategory {};
DEFINE_UNIT(Second, TimeCategory, 1.0);
DEFINE_UNIT(Minute, TimeCategory, 60.0);
struct HealthCategory {};
DEFINE_UNIT(HealthPoints, HealthCategory, 1.0);
struct ArmorCatergory {};
DEFINE_UNIT(Armor, ArmorCatergory, 1.0);

} // namespace Simulator
