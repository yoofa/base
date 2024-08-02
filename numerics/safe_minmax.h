/*
 * safe_minmax.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef SAFE_MINMAX_H
#define SAFE_MINMAX_H

#include <limits>
#include <type_traits>

#include "base/checks.h"
#include "base/numerics/safe_compare.h"
#include "base/type_traits.h"

namespace ave {
namespace base {
namespace safe_minmax_impl {

// Make the range of a type available via something other than a constexpr
// function, to work around MSVC limitations. See
// https://blogs.msdn.microsoft.com/vcblog/2015/12/02/partial-support-for-expression-sfinae-in-vs-2015-update-1/
template <typename T>
struct Limits {
  static constexpr T lowest = std::numeric_limits<T>::lowest();
  static constexpr T max = std::numeric_limits<T>::max();
};

template <typename T, bool is_enum = std::is_enum<T>::value>
struct UnderlyingType;

template <typename T>
struct UnderlyingType<T, false> {
  using type = T;
};

template <typename T>
struct UnderlyingType<T, true> {
  using type = typename std::underlying_type<T>::type;
};

// Given two types T1 and T2, find types that can hold the smallest (in
// ::min_t) and the largest (in ::max_t) of the two values.
template <typename T1,
          typename T2,
          bool int1 = IsIntlike<T1>::value,
          bool int2 = IsIntlike<T2>::value>
struct MType {
  static_assert(int1 == int2,
                "You may not mix integral and floating-point arguments");
};

// Specialization for when neither type is integral (and therefore presumably
// floating-point).
template <typename T1, typename T2>
struct MType<T1, T2, false, false> {
  using min_t = typename std::common_type<T1, T2>::type;
  static_assert(std::is_same<min_t, T1>::value ||
                    std::is_same<min_t, T2>::value,
                "");

  using max_t = typename std::common_type<T1, T2>::type;
  static_assert(std::is_same<max_t, T1>::value ||
                    std::is_same<max_t, T2>::value,
                "");
};

// Specialization for when both types are integral.
template <typename T1, typename T2>
struct MType<T1, T2, true, true> {
  // The type with the lowest minimum value. In case of a tie, the type with
  // the lowest maximum value. In case that too is a tie, the types have the
  // same range, and we arbitrarily pick T1.
  using min_t = typename std::conditional<
      SafeLt(Limits<T1>::lowest, Limits<T2>::lowest),
      T1,
      typename std::conditional<
          SafeGt(Limits<T1>::lowest, Limits<T2>::lowest),
          T2,
          typename std::conditional<SafeLe(Limits<T1>::max, Limits<T2>::max),
                                    T1,
                                    T2>::type>::type>::type;
  static_assert(std::is_same<min_t, T1>::value ||
                    std::is_same<min_t, T2>::value,
                "");

  // The type with the highest maximum value. In case of a tie, the types have
  // the same range (because in C++, integer types with the same maximum also
  // have the same minimum).
  static_assert(SafeNe(Limits<T1>::max, Limits<T2>::max) ||
                    SafeEq(Limits<T1>::lowest, Limits<T2>::lowest),
                "integer types with the same max should have the same min");
  using max_t = typename std::
      conditional<SafeGe(Limits<T1>::max, Limits<T2>::max), T1, T2>::type;
  static_assert(std::is_same<max_t, T1>::value ||
                    std::is_same<max_t, T2>::value,
                "");
};

// A dummy type that we pass around at compile time but never actually use.
// Declared but not defined.
struct DefaultType;

// ::type is A, except we fall back to B if A is DefaultType. We static_assert
// that the chosen type can hold all values that B can hold.
template <typename A, typename B>
struct TypeOr {
  using type = typename std::
      conditional<std::is_same<A, DefaultType>::value, B, A>::type;
  static_assert(SafeLe(Limits<type>::lowest, Limits<B>::lowest) &&
                    SafeGe(Limits<type>::max, Limits<B>::max),
                "The specified type isn't large enough");
  static_assert(IsIntlike<type>::value == IsIntlike<B>::value &&
                    std::is_floating_point<type>::value ==
                        std::is_floating_point<type>::value,
                "float<->int conversions not allowed");
};
}  // namespace safe_minmax_impl

template <
    typename R = safe_minmax_impl::DefaultType,
    typename T1 = safe_minmax_impl::DefaultType,
    typename T2 = safe_minmax_impl::DefaultType,
    typename R2 = typename safe_minmax_impl::TypeOr<
        R,
        typename safe_minmax_impl::MType<
            typename safe_minmax_impl::UnderlyingType<T1>::type,
            typename safe_minmax_impl::UnderlyingType<T2>::type>::min_t>::type>
constexpr R2 SafeMin(T1 a, T2 b) {
  static_assert(IsIntlike<T1>::value || std::is_floating_point<T1>::value,
                "The first argument must be integral or floating-point");
  static_assert(IsIntlike<T2>::value || std::is_floating_point<T2>::value,
                "The second argument must be integral or floating-point");
  return SafeLt(a, b) ? static_cast<R2>(a) : static_cast<R2>(b);
}

template <
    typename R = safe_minmax_impl::DefaultType,
    typename T1 = safe_minmax_impl::DefaultType,
    typename T2 = safe_minmax_impl::DefaultType,
    typename R2 = typename safe_minmax_impl::TypeOr<
        R,
        typename safe_minmax_impl::MType<
            typename safe_minmax_impl::UnderlyingType<T1>::type,
            typename safe_minmax_impl::UnderlyingType<T2>::type>::max_t>::type>
constexpr R2 SafeMax(T1 a, T2 b) {
  static_assert(IsIntlike<T1>::value || std::is_floating_point<T1>::value,
                "The first argument must be integral or floating-point");
  static_assert(IsIntlike<T2>::value || std::is_floating_point<T2>::value,
                "The second argument must be integral or floating-point");
  return SafeGt(a, b) ? static_cast<R2>(a) : static_cast<R2>(b);
}

namespace safe_minmax_impl {

// Given three types T, L, and H, let ::type be a suitable return value for
// SafeClamp(T, L, H). See the docs at the top of this file for details.
template <typename T,
          typename L,
          typename H,
          bool int1 = IsIntlike<T>::value,
          bool int2 = IsIntlike<L>::value,
          bool int3 = IsIntlike<H>::value>
struct ClampType {
  static_assert(int1 == int2 && int1 == int3,
                "You may not mix integral and floating-point arguments");
};

// Specialization for when all three types are floating-point.
template <typename T, typename L, typename H>
struct ClampType<T, L, H, false, false, false> {
  using type = typename std::common_type<T, L, H>::type;
};

// Specialization for when all three types are integral.
template <typename T, typename L, typename H>
struct ClampType<T, L, H, true, true, true> {
 private:
  // Range of the return value. The return type must be able to represent this
  // full range.
  static constexpr auto r_min =
      SafeMax(Limits<L>::lowest, SafeMin(Limits<H>::lowest, Limits<T>::lowest));
  static constexpr auto r_max =
      SafeMin(Limits<H>::max, SafeMax(Limits<L>::max, Limits<T>::max));

  // Is the given type an acceptable return type? (That is, can it represent
  // all possible return values, and is it no larger than the largest of the
  // input types?)
  template <typename A>
  struct AcceptableType {
   private:
    static constexpr bool not_too_large = sizeof(A) <= sizeof(L) ||
                                          sizeof(A) <= sizeof(H) ||
                                          sizeof(A) <= sizeof(T);
    static constexpr bool range_contained =
        SafeLe(Limits<A>::lowest, r_min) && SafeLe(r_max, Limits<A>::max);

   public:
    static constexpr bool value = not_too_large && range_contained;
  };

  using best_signed_type = typename std::conditional<
      AcceptableType<int8_t>::value,
      int8_t,
      typename std::conditional<
          AcceptableType<int16_t>::value,
          int16_t,
          typename std::conditional<AcceptableType<int32_t>::value,
                                    int32_t,
                                    int64_t>::type>::type>::type;

  using best_unsigned_type = typename std::conditional<
      AcceptableType<uint8_t>::value,
      uint8_t,
      typename std::conditional<
          AcceptableType<uint16_t>::value,
          uint16_t,
          typename std::conditional<AcceptableType<uint32_t>::value,
                                    uint32_t,
                                    uint64_t>::type>::type>::type;

 public:
  // Pick the best type, preferring the same signedness as T but falling back
  // to the other one if necessary.
  using type = typename std::conditional<
      std::is_signed<T>::value,
      typename std::conditional<AcceptableType<best_signed_type>::value,
                                best_signed_type,
                                best_unsigned_type>::type,
      typename std::conditional<AcceptableType<best_unsigned_type>::value,
                                best_unsigned_type,
                                best_signed_type>::type>::type;
  static_assert(AcceptableType<type>::value, "");
};

}  // namespace safe_minmax_impl

template <
    typename R = safe_minmax_impl::DefaultType,
    typename T = safe_minmax_impl::DefaultType,
    typename L = safe_minmax_impl::DefaultType,
    typename H = safe_minmax_impl::DefaultType,
    typename R2 = typename safe_minmax_impl::TypeOr<
        R,
        typename safe_minmax_impl::ClampType<
            typename safe_minmax_impl::UnderlyingType<T>::type,
            typename safe_minmax_impl::UnderlyingType<L>::type,
            typename safe_minmax_impl::UnderlyingType<H>::type>::type>::type>
R2 SafeClamp(T x, L min, H max) {
  static_assert(IsIntlike<H>::value || std::is_floating_point<H>::value,
                "The first argument must be integral or floating-point");
  static_assert(IsIntlike<T>::value || std::is_floating_point<T>::value,
                "The second argument must be integral or floating-point");
  static_assert(IsIntlike<L>::value || std::is_floating_point<L>::value,
                "The third argument must be integral or floating-point");
  AVE_DCHECK_LE(min, max);
  return SafeLe(x, min)   ? static_cast<R2>(min)
         : SafeGe(x, max) ? static_cast<R2>(max)
                          : static_cast<R2>(x);
}

}  // namespace base
}  // namespace ave

#endif /* !SAFE_MINMAX_H */
