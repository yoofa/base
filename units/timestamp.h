/*
 * timestamp.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "time_delta.h"
#include "unit_base.h"

namespace ave {
namespace base {
// Timestamp represents the time that has passed since some unspecified epoch.
// The epoch is assumed to be before any represented timestamps, this means that
// negative values are not valid. The most notable feature is that the
// difference of two Timestamps results in a TimeDelta.
class Timestamp : public unit_impl::UnitBase<Timestamp> {
 public:
  template <typename T>
  static constexpr Timestamp Seconds(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromFraction(1'000'000, value);
  }
  template <typename T>
  static constexpr Timestamp Millis(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromFraction(1'000, value);
  }
  template <typename T>
  static constexpr Timestamp Micros(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromValue(value);
  }

  Timestamp() = delete;

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

  constexpr int64_t seconds_or(int64_t fallback_value) const {
    return ToFractionOr<1000000>(fallback_value);
  }
  constexpr int64_t ms_or(int64_t fallback_value) const {
    return ToFractionOr<1000>(fallback_value);
  }
  constexpr int64_t us_or(int64_t fallback_value) const {
    return ToValueOr(fallback_value);
  }

  constexpr Timestamp operator+(const TimeDelta delta) const {
    if (IsPlusInfinity() || delta.IsPlusInfinity()) {
      AVE_DCHECK(!IsMinusInfinity());
      AVE_DCHECK(!delta.IsMinusInfinity());
      return PlusInfinity();
    }

    if (IsMinusInfinity() || delta.IsMinusInfinity()) {
      AVE_DCHECK(!IsPlusInfinity());
      AVE_DCHECK(!delta.IsPlusInfinity());
      return MinusInfinity();
    }

    return Timestamp::Micros(us() + delta.us());
  }

  constexpr Timestamp operator-(const TimeDelta delta) const {
    if (IsPlusInfinity() || delta.IsMinusInfinity()) {
      AVE_DCHECK(!IsMinusInfinity());
      AVE_DCHECK(!delta.IsPlusInfinity());
      return PlusInfinity();
    }

    if (IsMinusInfinity() || delta.IsPlusInfinity()) {
      AVE_DCHECK(!IsPlusInfinity());
      AVE_DCHECK(!delta.IsMinusInfinity());
      return MinusInfinity();
    }

    return Timestamp::Micros(us() - delta.us());
  }
  constexpr TimeDelta operator-(const Timestamp other) const {
    if (IsPlusInfinity() || other.IsMinusInfinity()) {
      AVE_DCHECK(!IsMinusInfinity());
      AVE_DCHECK(!other.IsPlusInfinity());
      return TimeDelta::PlusInfinity();
    }

    if (IsMinusInfinity() || other.IsPlusInfinity()) {
      AVE_DCHECK(!IsPlusInfinity());
      AVE_DCHECK(!other.IsMinusInfinity());
      return TimeDelta::MinusInfinity();
    }

    return TimeDelta::Micros(us() - other.us());
  }
  constexpr Timestamp& operator-=(const TimeDelta delta) {
    *this = *this - delta;
    return *this;
  }
  constexpr Timestamp& operator+=(const TimeDelta delta) {
    *this = *this + delta;
    return *this;
  }

 private:
  friend class unit_impl::UnitBase<Timestamp>;
  using UnitBase::UnitBase;
  static constexpr bool one_sided = true;
};

std::string ToString(Timestamp value);
inline std::string ToLogString(Timestamp value) {
  return ToString(value);
}

}  // namespace base
}  // namespace ave

#endif /* !TIMESTAMP_H */
