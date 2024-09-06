/*
 * divide_round.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef DIVIDE_ROUND_H
#define DIVIDE_ROUND_H

#include <type_traits>

#include "base/checks.h"
#include "base/numerics/safe_compare.h"

namespace ave {
namespace base {

template <typename Dividend, typename Divisor>
inline auto constexpr DivideRoundUp(Dividend dividend, Divisor divisor) {
  static_assert(std::is_integral_v<Dividend>, "Dividend must be integral");
  static_assert(std::is_integral_v<Divisor>, "Divisor must be integral");
  AVE_DCHECK_GE(dividend, 0);
  AVE_DCHECK_GT(divisor, 0);

  auto const quotient = dividend / divisor;
  auto const remainder = dividend % divisor;
  return quotient + (remainder > 0 ? 1 : 0);
}

template <typename Dividend, typename Divisor>
inline auto constexpr DivideRoundToNearest(Dividend dividend, Divisor divisor) {
  static_assert(std::is_integral_v<Dividend>, "Dividend must be integral");
  static_assert(std::is_integral_v<Divisor>, "Divisor must be integral");
  AVE_DCHECK_GT(divisor, 0);

  if (dividend < Dividend{0}) {
    auto const half_of_divisor = divisor / 2;
    auto quotient = dividend / divisor;
    auto const remainder = dividend % divisor;
    if (SafeGt(-remainder, half_of_divisor)) {
      --quotient;
    }
    return quotient;
  }

  auto const half_of_divisor = (divisor - 1) / 2;
  auto quotient = dividend / divisor;
  auto const remainder = dividend % divisor;
  if (SafeGt(remainder, half_of_divisor)) {
    ++quotient;
  }
  return quotient;
}

}  // namespace base
}  // namespace ave

#endif /* !DIVIDE_ROUND_H */
