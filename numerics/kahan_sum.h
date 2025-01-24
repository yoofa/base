/*
 * kahan_sum.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef KAHAN_SUM_H
#define KAHAN_SUM_H

namespace ave {
namespace base {

/**
 * Compensated summation is used to accumulate a sequence of floating point
 * values, with "compensation" information to help preserve precision lost
 * due to catastrophic cancellation, e.g. (BIG) + (SMALL) - (BIG) = 0.
 *
 * We provide two forms of compensated summation:
 * the Kahan variant (which has better properties if the sum is generally
 * larger than the data added; and the Neumaier variant which is better if
 * the sum or delta may alternatively be larger.
 *
 * https://en.wikipedia.org/wiki/Kahan_summation_algorithm
 *
 * Alternative approaches include divide-and-conquer summation
 * which provides increased accuracy with log n stack depth (recursion).
 *
 * https://en.wikipedia.org/wiki/Pairwise_summation
 */

// KahanSum provides compensated summation to help preserve precision lost
// due to catastrophic cancellation in floating point arithmetic.
// For example: (BIG) + (SMALL) - (BIG) = 0 due to precision loss.
template <typename T>
class KahanSum {
 public:
  constexpr KahanSum() = default;
  explicit constexpr KahanSum(const T& value) : sum_(value) {}

  friend constexpr KahanSum operator+(KahanSum lhs, const T& rhs) {
    const T y = rhs - lhs.correction_;
    const T t = lhs.sum_ + y;
    lhs.correction_ = (t - lhs.sum_) - y;
    lhs.sum_ = t;
    return lhs;
  }

  constexpr KahanSum& operator+=(const T& rhs) {
    *this = *this + rhs;
    return *this;
  }

  constexpr operator T() const { return sum_; }

  constexpr void Reset() {
    sum_ = T{};
    correction_ = T{};
  }

 private:
  T sum_{};         // running sum
  T correction_{};  // running error compensation
};

// A more robust version of Kahan summation for input greater than sum.
template <typename T>
class NeumaierSum {
 public:
  constexpr NeumaierSum() = default;
  explicit constexpr NeumaierSum(const T& value) : sum_(value) {}

  friend constexpr NeumaierSum operator+(NeumaierSum lhs, const T& rhs) {
    const T t = lhs.sum_ + rhs;
    if (ConstAbs(lhs.sum_) >= ConstAbs(rhs)) {
      lhs.correction_ += (lhs.sum_ - t) + rhs;
    } else {
      lhs.correction_ += (rhs - t) + lhs.sum_;
    }
    lhs.sum_ = t;
    return lhs;
  }

  constexpr NeumaierSum& operator+=(const T& rhs) {
    *this = *this + rhs;
    return *this;
  }

  constexpr operator T() const { return sum_ + correction_; }

  constexpr void Reset() {
    sum_ = T{};
    correction_ = T{};
  }

 private:
  static constexpr T ConstAbs(T x) { return x < T{} ? -x : x; }

  T sum_{};         // running sum
  T correction_{};  // running error compensation
};

}  // namespace base
}  // namespace ave

#endif /* !KAHAN_SUM_H */