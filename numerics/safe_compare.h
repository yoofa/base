/*
 * safe_compare.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef SAFE_COMPARE_H
#define SAFE_COMPARE_H

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

#include "base/type_traits.h"

namespace avp {
namespace base {
namespace safe_cmp_impl {

template <size_t N>
struct LargerIntImpl : std::false_type {};
template <>
struct LargerIntImpl<sizeof(int8_t)> : std::true_type {
  using type = int16_t;
};
template <>
struct LargerIntImpl<sizeof(int16_t)> : std::true_type {
  using type = int32_t;
};
template <>
struct LargerIntImpl<sizeof(int32_t)> : std::true_type {
  using type = int64_t;
};

// LargerInt<T1, T2>::value is true iff there's a signed type that's larger
// than T1 (and no larger than the larger of T2 and int*, for performance
// reasons); and if there is such a type, LargerInt<T1, T2>::type is an alias
// for it.
template <typename T1, typename T2>
struct LargerInt
    : LargerIntImpl<sizeof(T1) < sizeof(T2) || sizeof(T1) < sizeof(int*)
                        ? sizeof(T1)
                        : 0> {};

template <typename T>
constexpr typename std::make_unsigned<T>::type MakeUnsigned(T a) {
  return static_cast<typename std::make_unsigned<T>::type>(a);
}

// Overload for when both T1 and T2 have the same signedness.
template <typename Op,
          typename T1,
          typename T2,
          typename std::enable_if<std::is_signed<T1>::value ==
                                  std::is_signed<T2>::value>::type* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return Op::Op(a, b);
}

// Overload for signed - unsigned comparison that can be promoted to a bigger
// signed type.
template <typename Op,
          typename T1,
          typename T2,
          typename std::enable_if<std::is_signed<T1>::value &&
                                  std::is_unsigned<T2>::value &&
                                  LargerInt<T2, T1>::value>::type* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return Op::Op(a, static_cast<typename LargerInt<T2, T1>::type>(b));
}

// Overload for unsigned - signed comparison that can be promoted to a bigger
// signed type.
template <typename Op,
          typename T1,
          typename T2,
          typename std::enable_if<std::is_unsigned<T1>::value &&
                                  std::is_signed<T2>::value &&
                                  LargerInt<T1, T2>::value>::type* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return Op::Op(static_cast<typename LargerInt<T1, T2>::type>(a), b);
}

// Overload for signed - unsigned comparison that can't be promoted to a bigger
// signed type.
template <typename Op,
          typename T1,
          typename T2,
          typename std::enable_if<std::is_signed<T1>::value &&
                                  std::is_unsigned<T2>::value &&
                                  !LargerInt<T2, T1>::value>::type* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return a < 0 ? Op::Op(-1, 0) : Op::Op(safe_cmp_impl::MakeUnsigned(a), b);
}

// Overload for unsigned - signed comparison that can't be promoted to a bigger
// signed type.
template <typename Op,
          typename T1,
          typename T2,
          typename std::enable_if<std::is_unsigned<T1>::value &&
                                  std::is_signed<T2>::value &&
                                  !LargerInt<T1, T2>::value>::type* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return b < 0 ? Op::Op(0, -1) : Op::Op(a, safe_cmp_impl::MakeUnsigned(b));
}

#define AVP_SAFECMP_MAKE_OP(name, op)      \
  struct name {                            \
    template <typename T1, typename T2>    \
    static constexpr bool Op(T1 a, T2 b) { \
      return a op b;                       \
    }                                      \
  };
AVP_SAFECMP_MAKE_OP(EqOp, ==)
AVP_SAFECMP_MAKE_OP(NeOp, !=)
AVP_SAFECMP_MAKE_OP(LtOp, <)
AVP_SAFECMP_MAKE_OP(LeOp, <=)
AVP_SAFECMP_MAKE_OP(GtOp, >)
AVP_SAFECMP_MAKE_OP(GeOp, >=)
#undef AVP_SAFECMP_MAKE_OP

}  // namespace safe_cmp_impl

#define AVP_SAFECMP_MAKE_FUN(name)                                            \
  template <typename T1, typename T2>                                         \
  constexpr                                                                   \
      typename std::enable_if<IsIntlike<T1>::value && IsIntlike<T2>::value,   \
                              bool>::type Safe##name(T1 a, T2 b) {            \
    /* Unary plus here turns enums into real integral types. */               \
    return safe_cmp_impl::Cmp<safe_cmp_impl::name##Op>(+a, +b);               \
  }                                                                           \
  template <typename T1, typename T2>                                         \
  constexpr                                                                   \
      typename std::enable_if<!IsIntlike<T1>::value || !IsIntlike<T2>::value, \
                              bool>::type Safe##name(const T1& a,             \
                                                     const T2& b) {           \
    return safe_cmp_impl::name##Op::Op(a, b);                                 \
  }

AVP_SAFECMP_MAKE_FUN(Eq)
AVP_SAFECMP_MAKE_FUN(Ne)
AVP_SAFECMP_MAKE_FUN(Lt)
AVP_SAFECMP_MAKE_FUN(Le)
AVP_SAFECMP_MAKE_FUN(Gt)
AVP_SAFECMP_MAKE_FUN(Ge)
#undef AVP_SAFECMP_MAKE_FUN

}  // namespace base
}  // namespace avp

#endif /* !SAFE_COMPARE_H */
