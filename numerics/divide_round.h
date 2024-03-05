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

namespace avp {
namespace base {

template <typename Dividend, typename Divisor>
inline auto constexpr DivideRoundUp(Dividend dividend, Divisor divisor) {
  static_assert(std::is_integral<Dividend>(), "");
  static_assert(std::is_integral<Divisor>(), "");
  DCHECK_GE(dividend, 0);
  DCHECK_GT(divisor, 0);

  auto quotient = dividend / divisor;
  auto remainder = dividend % divisor;
  return quotient + (remainder > 0 ? 1 : 0);
}

template <typename Dividend, typename Divisor>
inline auto constexpr DivideRoundToNearest(Dividend dividend, Divisor divisor) {
  static_assert(std::is_integral<Dividend>(), "");
  static_assert(std::is_integral<Divisor>(), "");
  DCHECK_GT(divisor, 0);

  if (dividend < Dividend{0}) {
    auto half_of_divisor = divisor / 2;
    auto quotient = dividend / divisor;
    auto remainder = dividend % divisor;
    if (SafeGt(-remainder, half_of_divisor)) {
      --quotient;
    }
    return quotient;
  }

  auto half_of_divisor = (divisor - 1) / 2;
  auto quotient = dividend / divisor;
  auto remainder = dividend % divisor;
  if (SafeGt(remainder, half_of_divisor)) {
    ++quotient;
  }
  return quotient;
}

}  // namespace base
}  // namespace avp

#endif /* !DIVIDE_ROUND_H */
