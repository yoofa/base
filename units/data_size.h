/*
 * data_size.h - Data size unit class
 * Ported from WebRTC (api/units/data_size.h)
 *
 * Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the root of the source tree. An additional
 * intellectual property rights grant can be found in the file PATENTS.
 */

#ifndef BASE_UNITS_DATA_SIZE_H_
#define BASE_UNITS_DATA_SIZE_H_

#include <cstdint>
#include <string>
#include <type_traits>

#include "base/units/unit_base.h"

namespace ave {
namespace base {

// DataSize is a class represeting a count of bytes.
class DataSize final : public base::unit_impl::RelativeUnit<DataSize> {
 public:
  template <typename T>
  static constexpr DataSize Bytes(T value) {
    static_assert(std::is_arithmetic_v<T>);
    return FromValue(value);
  }
  static constexpr DataSize Infinity() { return PlusInfinity(); }

  DataSize() = delete;

  template <typename T = int64_t>
  constexpr T bytes() const {
    return ToValue<T>();
  }

  constexpr int64_t bytes_or(int64_t fallback_value) const {
    return ToValueOr(fallback_value);
  }

 private:
  friend class base::unit_impl::UnitBase<DataSize>;
  using RelativeUnit::RelativeUnit;
  static constexpr bool one_sided = true;
};

std::string ToString(DataSize value);

}  // namespace base
}  // namespace ave

#endif  // BASE_UNITS_DATA_SIZE_H_
