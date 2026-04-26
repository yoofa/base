/*
 * frequency.h - Frequency unit class
 *
 * Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 * Adapted for aspect video engine.
 */

#ifndef BASE_UNITS_FREQUENCY_H_
#define BASE_UNITS_FREQUENCY_H_

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <type_traits>

#include "base/checks.h"
#include "base/units/time_delta.h"
#include "base/units/unit_base.h"

namespace ave {
namespace base {

// Forward declare to use in operators below
using TimeDelta = ::ave::base::TimeDelta;

class Frequency final : public base::unit_impl::RelativeUnit<Frequency> {
 public:
  template <typename T>
  static constexpr Frequency MilliHertz(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromValue(value);
  }
  template <typename T>
  static constexpr Frequency Hertz(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromFraction(1'000, value);
  }
  template <typename T>
  static constexpr Frequency KiloHertz(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromFraction(1'000'000, value);
  }

  constexpr Frequency() = delete;

  template <typename T = int64_t>
  constexpr T hertz() const {
    return ToFraction<1000, T>();
  }
  template <typename T = int64_t>
  constexpr T millihertz() const {
    return ToValue<T>();
  }

 private:
  friend class base::unit_impl::UnitBase<Frequency>;
  using RelativeUnit::RelativeUnit;
  static constexpr bool one_sided = true;
};

inline constexpr Frequency operator/(int64_t nominator,
                                     const TimeDelta& interval) {
  constexpr int64_t kKiloPerMicro = 1000 * 1000000;
  AVE_DCHECK_LE(nominator, std::numeric_limits<int64_t>::max() / kKiloPerMicro);
  AVE_CHECK(interval.IsFinite());
  AVE_CHECK(!interval.IsZero());
  return Frequency::MilliHertz(nominator * kKiloPerMicro / interval.us());
}

inline constexpr TimeDelta operator/(int64_t nominator,
                                     const Frequency& frequency) {
  constexpr int64_t kMegaPerMilli = 1000000 * 1000;
  AVE_DCHECK_LE(nominator, std::numeric_limits<int64_t>::max() / kMegaPerMilli);
  AVE_CHECK(frequency.IsFinite());
  AVE_CHECK(!frequency.IsZero());
  return TimeDelta::Micros(nominator * kMegaPerMilli / frequency.millihertz());
}

inline constexpr double operator*(Frequency frequency, TimeDelta time_delta) {
  return frequency.hertz<double>() * time_delta.seconds<double>();
}
inline constexpr double operator*(TimeDelta time_delta, Frequency frequency) {
  return frequency * time_delta;
}

std::string ToString(Frequency value);

}  // namespace base
}  // namespace ave

#endif  // BASE_UNITS_FREQUENCY_H_
