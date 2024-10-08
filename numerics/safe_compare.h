/*
 * safe_compare.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef SAFE_COMPARE_H
#define SAFE_COMPARE_H

#include <cstddef>
#include <cstdint>

#include <type_traits>

#include "base/type_traits.h"

namespace ave {
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
constexpr auto MakeUnsigned(T a) {
  return static_cast<std::make_unsigned_t<T>>(a);
}

// Overload for when both T1 and T2 have the same signedness.
template <typename Op,
          typename T1,
          typename T2,
          typename std::enable_if_t<std::is_signed_v<T1> ==
                                    std::is_signed_v<T2>>* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return Op::Op(a, b);
}

// Overload for signed - unsigned comparison that can be promoted to a bigger
// signed type.
template <
    typename Op,
    typename T1,
    typename T2,
    typename std::enable_if_t<std::is_signed_v<T1> && std::is_unsigned_v<T2> &&
                              LargerInt<T2, T1>::value>* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return Op::Op(a, static_cast<typename LargerInt<T2, T1>::type>(b));
}

// Overload for unsigned - signed comparison that can be promoted to a bigger
// signed type.
template <
    typename Op,
    typename T1,
    typename T2,
    typename std::enable_if_t<std::is_unsigned_v<T1> && std::is_signed_v<T2> &&
                              LargerInt<T1, T2>::value>* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return Op::Op(static_cast<typename LargerInt<T1, T2>::type>(a), b);
}

// Overload for signed - unsigned comparison that can't be promoted to a bigger
// signed type.
template <
    typename Op,
    typename T1,
    typename T2,
    typename std::enable_if_t<std::is_signed_v<T1> && std::is_unsigned_v<T2> &&
                              !LargerInt<T2, T1>::value>* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return a < 0 ? Op::Op(-1, 0) : Op::Op(safe_cmp_impl::MakeUnsigned(a), b);
}

// Overload for unsigned - signed comparison that can't be promoted to a bigger
// signed type.
template <
    typename Op,
    typename T1,
    typename T2,
    typename std::enable_if_t<std::is_unsigned_v<T1> && std::is_signed_v<T2> &&
                              !LargerInt<T1, T2>::value>* = nullptr>
constexpr bool Cmp(T1 a, T2 b) {
  return b < 0 ? Op::Op(0, -1) : Op::Op(a, safe_cmp_impl::MakeUnsigned(b));
}

#define AVE_SAFECMP_MAKE_OP(name, op)      \
  struct name {                            \
    template <typename T1, typename T2>    \
    static constexpr bool Op(T1 a, T2 b) { \
      return a op b;                       \
    }                                      \
  };
AVE_SAFECMP_MAKE_OP(EqOp, ==)
AVE_SAFECMP_MAKE_OP(NeOp, !=)
AVE_SAFECMP_MAKE_OP(LtOp, <)
AVE_SAFECMP_MAKE_OP(LeOp, <=)
AVE_SAFECMP_MAKE_OP(GtOp, >)
AVE_SAFECMP_MAKE_OP(GeOp, >=)
#undef AVE_SAFECMP_MAKE_OP

}  // namespace safe_cmp_impl

#define AVE_SAFECMP_MAKE_FUN(name)                                           \
  template <typename T1, typename T2>                                        \
  constexpr std::enable_if_t<IsIntlike<T1>::value && IsIntlike<T2>::value,   \
                             bool>                                           \
      Safe##name(T1 a, T2 b) {                                               \
    /* Unary plus here turns enums into real integral types. */              \
    return safe_cmp_impl::Cmp<safe_cmp_impl::name##Op>(+a, +b);              \
  }                                                                          \
  template <typename T1, typename T2>                                        \
  constexpr std::enable_if_t<!IsIntlike<T1>::value || !IsIntlike<T2>::value, \
                             bool>                                           \
      Safe##name(const T1& a, const T2& b) {                                 \
    return safe_cmp_impl::name##Op::Op(a, b);                                \
  }

AVE_SAFECMP_MAKE_FUN(Eq)
AVE_SAFECMP_MAKE_FUN(Ne)
AVE_SAFECMP_MAKE_FUN(Lt)
AVE_SAFECMP_MAKE_FUN(Le)
AVE_SAFECMP_MAKE_FUN(Gt)
AVE_SAFECMP_MAKE_FUN(Ge)
#undef AVE_SAFECMP_MAKE_FUN

}  // namespace base
}  // namespace ave

#endif /* !SAFE_COMPARE_H */
