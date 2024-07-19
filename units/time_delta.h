/*
 * time_delta.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef TIME_DELTA_H
#define TIME_DELTA_H

#include <cstdlib>
#include <string>
#include <type_traits>

#include "base/units/unit_base.h"

namespace ave {
namespace base {

// TimeDelta represents the difference between two timestamps. Commonly this can
// be a duration. However since two Timestamps are not guaranteed to have the
// same epoch (they might come from different computers, making exact
// synchronisation infeasible), the duration covered by a TimeDelta can be
// undefined. To simplify usage, it can be constructed and converted to
// different units, specifically seconds (s), milliseconds (ms) and
// microseconds (us).
class TimeDelta final : public unit_impl::RelativeUnit<TimeDelta> {
 public:
  template <typename T>
  static constexpr TimeDelta Minutes(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return Seconds(value * 60);
  }
  template <typename T>
  static constexpr TimeDelta Seconds(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromFraction(1'000'000, value);
  }
  template <typename T>
  static constexpr TimeDelta Millis(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromFraction(1'000, value);
  }
  template <typename T>
  static constexpr TimeDelta Micros(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromValue(value);
  }

  TimeDelta() = delete;

  template <typename T = int64_t>
  constexpr T seconds() const {
    return ToFraction<1000000, T>();
  }
  template <typename T = int64_t>
  constexpr T ms() const {
    return ToFraction<1000, T>();
  }
  template <typename T = int64_t>
  constexpr T us() const {
    return ToValue<T>();
  }
  template <typename T = int64_t>
  constexpr T ns() const {
    return ToMultiple<1000, T>();
  }

  constexpr int64_t seconds_or(int64_t fallback_value) const {
    return ToFractionOr<1000000>(fallback_value);
  }
  constexpr int64_t ms_or(int64_t fallback_value) const {
    return ToFractionOr<1000>(fallback_value);
  }
  constexpr int64_t us_or(int64_t fallback_value) const {
    return ToValueOr(fallback_value);
  }

  constexpr TimeDelta Abs() const {
    return us() < 0 ? TimeDelta::Micros(-us()) : *this;
  }

 private:
  friend class unit_impl::UnitBase<TimeDelta>;
  using RelativeUnit::RelativeUnit;
  static constexpr bool one_sided = false;
};

std::string ToString(TimeDelta value);
inline std::string ToLogString(TimeDelta value) {
  return ToString(value);
}

}  // namespace base
}  // namespace ave

#endif /* !TIME_DELTA_H */
