/*
 * statistics.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_STATISTICS_H
#define AVE_STATISTICS_H

#include <array>
#include <cmath>
#include <deque>
#include <functional>
#include <limits>
#include <string>
#include <tuple>

#include "kahan_sum.h"

namespace ave {
namespace base {

// NOLINTBEGIN(modernize-avoid-c-arrays)

// Constants and limits for statistics
template <typename T, typename T2 = void>
struct StatisticsConstants;

template <typename T>
struct StatisticsConstants<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
  static constexpr T NegativeInfinity() {
    return std::numeric_limits<T>::has_infinity
               ? -std::numeric_limits<T>::infinity()
               : std::numeric_limits<T>::min();
  }

  static constexpr T kNegativeInfinity = NegativeInfinity();

  static constexpr T PositiveInfinity() {
    return std::numeric_limits<T>::has_infinity
               ? std::numeric_limits<T>::infinity()
               : std::numeric_limits<T>::max();
  }

  static constexpr T kPositiveInfinity = PositiveInfinity();
};

template <typename T>
struct StatisticsConstants<T, std::enable_if_t<!std::is_arithmetic_v<T>>> {
 private:
  template <std::size_t... I>
  static constexpr auto NegativeInfinity(std::index_sequence<I...>) {
    return T{StatisticsConstants<
        typename std::tuple_element<I, T>::type>::kNegativeInfinity...};
  }

  template <std::size_t... I>
  static constexpr auto PositiveInfinity(std::index_sequence<I...>) {
    return T{StatisticsConstants<
        typename std::tuple_element<I, T>::type>::kPositiveInfinity...};
  }

 public:
  static constexpr auto NegativeInfinity() {
    return NegativeInfinity(
        std::make_index_sequence<std::tuple_size<T>::value>());
  }

  static constexpr auto kNegativeInfinity =
      NegativeInfinity(std::make_index_sequence<std::tuple_size<T>::value>());

  static constexpr auto PositiveInfinity() {
    return PositiveInfinity(
        std::make_index_sequence<std::tuple_size<T>::value>());
  }

  static constexpr auto kPositiveInfinity =
      PositiveInfinity(std::make_index_sequence<std::tuple_size<T>::value>());
};

// Statistics provides running weighted average, variance, and standard
// deviation of a sample stream. It uses Welford's online algorithm with
// optional exponential weighting.
//
// The weighting is like an IIR filter, with the most recent sample weighted as
// 1, and decaying by alpha (between 0 and 1). With alpha == 1, this is
// rectangular weighting, reducing to Welford's algorithm.
//
// weight = sum_{i=1}^n \alpha^{n-i}
// mean = 1/weight * sum_{i=1}^n \alpha^{n-i}x_i
// var = 1/weight * sum_{i=1}^n alpha^{n-i}(x_i- mean)^2
template <typename T,                             // input data type
          typename D = double,                    // output mean type
          typename S = KahanSum<D>,               // summation type
          typename A = double,                    // weight type
          typename D2 = double,                   // output variance type
          typename PRODUCT = std::multiplies<D>>  // variance computation method
class Statistics {
 public:
  explicit constexpr Statistics(A alpha = A(1.)) : alpha_(alpha) {}

  template <size_t N>
  explicit constexpr Statistics(const T (&a)[N], A alpha = A(1.))
      : alpha_(alpha) {
    for (const auto& data : a) {
      Add(data);
    }
  }

  constexpr void SetAlpha(A alpha) { alpha_ = alpha; }

  constexpr void Add(const T& value) {
    max_ = std::max(max_, value);
    min_ = std::min(min_, value);
    ++count_;

    const D delta = value - mean_;
    weight_ = A(1.) + alpha_ * weight_;
    weight2_ = A(1.) + alpha_ * alpha_ * weight2_;
    D mean_delta = delta / weight_;
    mean_ += mean_delta;
    m2_ = alpha_ * m2_ + PRODUCT()(delta, (value - mean_));
  }

  constexpr int64_t Count() const { return count_; }

  constexpr void Reset() {
    min_ = StatisticsConstants<T>::kPositiveInfinity;
    max_ = StatisticsConstants<T>::kNegativeInfinity;
    count_ = 0;
    weight_ = {};
    weight2_ = {};
    mean_ = {};
    m2_ = {};
  }

  constexpr A Weight() const { return weight_; }

  constexpr D Mean() const { return mean_; }

  constexpr D2 Variance() const {
    if (count_ < 2) {
      return {};
    }
    return m2_ / GetSampleWeight();
  }

  constexpr D2 PopulationVariance() const {
    if (count_ < 1) {
      return {};
    }
    return m2_ / weight_;
  }

  D2 StdDev() const { return std::sqrt(Variance()); }

  D2 PopulationStdDev() const { return std::sqrt(PopulationVariance()); }

  constexpr T Min() const { return min_; }

  constexpr T Max() const { return max_; }

  std::string ToString() const {
    if (count_ == 0) {
      return "unavail";
    }

    std::string result = "ave=" + std::to_string(Mean());
    if (count_ > 1) {
      result += " std=" + std::to_string(StdDev());
    }
    result += " min=" + std::to_string(Min());
    result += " max=" + std::to_string(Max());
    return result;
  }

 private:
  // Reliability correction for unbiasing variance, since mean is estimated
  // from same sample stream as variance.
  constexpr A GetSampleWeight() const { return weight_ - weight2_ / weight_; }

  A alpha_;  // exponential weighting factor
  T min_{StatisticsConstants<T>::kPositiveInfinity};
  T max_{StatisticsConstants<T>::kNegativeInfinity};
  int64_t count_{0};  // number of samples
  A weight_{};        // sum of weights
  A weight2_{};       // sum of weights squared
  S mean_{};          // running mean
  D2 m2_{};           // running unnormalized variance
};

/**
 * Least squares fitting of a 2D straight line based on the covariance matrix.
 *
 * See formula from:
 * http://mathworld.wolfram.com/LeastSquaresFitting.html
 *
 * y = a + b*x
 *
 * returns a: y intercept
 *         b: slope
 *         r2: correlation coefficient (1.0 means great fit, 0.0 means no fit.)
 *
 * For better numerical stability, it is suggested to use the slope b only:
 * as the least squares fit line intersects the mean.
 *
 * (y - mean_y) = b * (x - mean_x).
 *
 */
template <typename T>
constexpr void computeYLineFromStatistics(T& a,
                                          T& b,
                                          T& r2,
                                          const T& mean_x,
                                          const T& mean_y,
                                          const T& var_x,
                                          const T& cov_xy,
                                          const T& var_y) {
  // Dimensionally r2 is unitless.  If there is no correlation
  // then r2 is clearly 0 as cov_xy == 0.  If x and y are identical up to a
  // scale and shift, then r2 is 1.
  r2 = cov_xy * cov_xy / (var_x * var_y);

  // The least squares solution to the overconstrained matrix equation requires
  // the pseudo-inverse. In 2D, the best-fit slope is the mean removed
  // (via covariance and variance) dy/dx derived from the joint expectation
  // (this is also dimensionally correct).
  b = cov_xy / var_x;

  // The best fit line goes through the mean, and can be used to find the y
  // intercept.
  a = mean_y - b * mean_x;
}

/**
 * LinearLeastSquaresFit<> class is derived from the Statistics<> class, with a
 * 2 element array. Arrays are preferred over tuples or pairs because copy
 * assignment is constexpr and arrays are trivially copyable.
 */
template <typename T>
class LinearLeastSquaresFit
    : public Statistics<std::array<T, 2>,  // input
                        std::array<T, 2>,  // mean data output
                        std::array<T, 2>,  // compensated mean sum
                        T,                 // weight type
                        std::array<T, 3>,  // covariance_ut
                        audio_utils::outerProduct_UT_array<std::array<T, 2>>> {
 public:
  constexpr explicit LinearLeastSquaresFit(const T& alpha = T(1.))
      : Statistics<std::array<T, 2>,
                   std::array<T, 2>,
                   std::array<T, 2>,
                   T,
                   std::array<T, 3>,  // covariance_ut
                   audio_utils::outerProduct_UT_array<std::array<T, 2>>>(
            alpha) {}

  /* Note: base class method: add(value)

  constexpr void add(const std::array<T, 2>& value);

     use:
        add({1., 2.});
     or
        add(to_array(myTuple));
  */

  /**
   * y = a + b*x
   *
   * returns a: y intercept
   *         b: y slope (dy / dx)
   *         r2: correlation coefficient (1.0 means great fit, 0.0 means no
   * fit.)
   */
  constexpr void computeYLine(T& a, T& b, T& r2) const {
    computeYLineFromStatistics(a, b, r2,
                               std::get<0>(this->getMean()),        /* mean_x */
                               std::get<1>(this->getMean()),        /* mean_y */
                               std::get<0>(this->getPopVariance()), /* var_x */
                               std::get<1>(this->getPopVariance()), /* cov_xy */
                               std::get<2>(this->getPopVariance())); /* var_y */
  }

  /**
   * x = a + b*y
   *
   * returns a: x intercept
   *         b: x slope (dx / dy)
   *         r2: correlation coefficient (1.0 means great fit, 0.0 means no
   * fit.)
   */
  constexpr void computeXLine(T& a, T& b, T& r2) const {
    // reverse x and y for X line computation
    computeYLineFromStatistics(a, b, r2,
                               std::get<1>(this->getMean()),        /* mean_x */
                               std::get<0>(this->getMean()),        /* mean_y */
                               std::get<2>(this->getPopVariance()), /* var_x */
                               std::get<1>(this->getPopVariance()), /* cov_xy */
                               std::get<0>(this->getPopVariance())); /* var_y */
  }

  /**
   * this returns the estimate of y from a given x
   */
  constexpr T getYFromX(const T& x) const {
    const T var_x = std::get<0>(this->getPopVariance());
    const T cov_xy = std::get<1>(this->getPopVariance());
    const T b = cov_xy / var_x;  // dy / dx

    const T mean_x = std::get<0>(this->getMean());
    const T mean_y = std::get<1>(this->getMean());
    return /* y = */ b * (x - mean_x) + mean_y;
  }

  /**
   * this returns the estimate of x from a given y
   */
  constexpr T getXFromY(const T& y) const {
    const T cov_xy = std::get<1>(this->getPopVariance());
    const T var_y = std::get<2>(this->getPopVariance());
    const T b = cov_xy / var_y;  // dx / dy

    const T mean_x = std::get<0>(this->getMean());
    const T mean_y = std::get<1>(this->getMean());
    return /* x = */ b * (y - mean_y) + mean_x;
  }

  constexpr T getR2() const {
    const T var_x = std::get<0>(this->getPopVariance());
    const T cov_xy = std::get<1>(this->getPopVariance());
    const T var_y = std::get<2>(this->getPopVariance());
    return cov_xy * cov_xy / (var_x * var_y);
  }
};

/**
 * constexpr statistics functions of form:
 * algorithm(forward_iterator begin, forward_iterator end)
 *
 * These check that the input looks like an iterator, but doesn't
 * check if __is_forward_iterator<>.
 *
 * divide-and-conquer pairwise summation forms will require
 * __is_random_access_iterator<>.
 */

/* is_iterator<T>::value is true if T supports std::iterator_traits<T>
   TODO: poor resolution on iterator type, prefer emulating hidden STL templates
   __is_input_iterator<>
   __is_forward_iterator<>
   ...
 */
// helper
struct is_iterator_impl {
  // SFINAE test(0) preferred if iterator traits
  template <typename T,
            typename = typename std::iterator_traits<T>::difference_type,
            typename = typename std::iterator_traits<T>::value_type,
            typename = typename std::iterator_traits<T>::pointer,
            typename = typename std::iterator_traits<T>::iterator_category>
  static int test(int);
  template <typename T>
  static bool test(...);
};

template <typename T>
struct is_iterator
    : std::integral_constant<
          bool,
          std::is_same_v<decltype(is_iterator_impl::test<std::decay_t<T>>(0)),
                         int>> {};

// returns max of elements, or if no elements negative infinity.
template <typename T, std::enable_if_t<is_iterator<T>::value, int> = 0>
constexpr auto max(T begin, T end) {
  using S = std::remove_cv_t<std::remove_reference_t<decltype(*begin)>>;
  S maxValue = StatisticsConstants<S>::mNegativeInfinity;
  for (auto it = begin; it != end; ++it) {
    maxValue = std::max(maxValue, *it);
  }
  return maxValue;
}

// returns min of elements, or if no elements positive infinity.
template <typename T, std::enable_if_t<is_iterator<T>::value, int> = 0>
constexpr auto min(T begin, T end) {
  using S = std::remove_cv_t<std::remove_reference_t<decltype(*begin)>>;
  S minValue = StatisticsConstants<S>::mPositiveInfinity;
  for (auto it = begin; it != end; ++it) {
    minValue = std::min(minValue, *it);
  }
  return minValue;
}

template <typename D = double,
          typename S = KahanSum<D>,
          typename T,
          std::enable_if_t<is_iterator<T>::value, int> = 0>
constexpr auto sum(T begin, T end) {
  S sum{};
  for (auto it = begin; it != end; ++it) {
    sum += D(*it);
  }
  return sum;
}

template <typename D = double,
          typename S = KahanSum<D>,
          typename T,
          std::enable_if_t<is_iterator<T>::value, int> = 0>
constexpr auto sumSqDiff(T begin, T end, D x = {}) {
  S sum{};
  for (auto it = begin; it != end; ++it) {
    const D diff = *it - x;
    sum += diff * diff;
  }
  return sum;
}

// Form: algorithm(array[]), where array size is known to the compiler.
template <typename T, size_t N>
constexpr T max(const T (&a)[N]) {
  return max(&a[0], &a[N]);
}

template <typename T, size_t N>
constexpr T min(const T (&a)[N]) {
  return min(&a[0], &a[N]);
}

template <typename D = double, typename S = KahanSum<D>, typename T, size_t N>
constexpr D sum(const T (&a)[N]) {
  return sum<D, S>(&a[0], &a[N]);
}

template <typename D = double, typename S = KahanSum<D>, typename T, size_t N>
constexpr D sumSqDiff(const T (&a)[N], D x = {}) {
  return sumSqDiff<D, S>(&a[0], &a[N], x);
}

// TODO: remove when std::isnan is constexpr
template <typename T>
constexpr T isnan(T x) {
  return __builtin_isnan(x);
}

// constexpr sqrt computed by the Babylonian (Newton's) method.
// Please use math libraries for non-constexpr cases.
// TODO: remove when there is some std::sqrt which is constexpr.
//
// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots

// watch out using the unchecked version, use the checked version below.
template <typename T>
constexpr T sqrt_constexpr_unchecked(T x, T prev) {
  static_assert(std::is_floating_point_v<T>, "must be floating point type");
  const T next = T(0.5) * (prev + x / prev);
  return next == prev ? next : sqrt_constexpr_unchecked(x, next);
}

// checked sqrt
template <typename T>
constexpr T sqrt_constexpr(T x) {
  static_assert(std::is_floating_point_v<T>, "must be floating point type");
  if (x < T{}) {  // negative values return nan
    return std::numeric_limits<T>::quiet_NaN();
  }
  if (isnan(x) || x == std::numeric_limits<T>::infinity() || x == T{}) {
    return x;
  }
  // good to go.
  return sqrt_constexpr_unchecked(x, T(1.));
}

/**
 * ReferenceStatistics is a naive implementation of the weighted running
 * variance, which consumes more space and is slower than Statistics. It is
 * provided for comparison and testing purposes. Do not call from a SCHED_FIFO
 * thread!
 *
 * Note: Common code not combined for implementation clarity.
 *       We don't invoke Kahan summation or other tricks.
 */
template <typename T,           // input data type
          typename D = double>  // output mean/variance data type
class ReferenceStatistics {
 public:
  /** alpha is the weight (alpha == 1. is rectangular window) */
  explicit constexpr ReferenceStatistics(D alpha = D(1.)) : alpha_(alpha) {}

  constexpr void SetAlpha(D alpha) { alpha_ = alpha; }

  // For independent testing, have intentionally slightly different behavior
  // of min and max than Statistics with respect to NaN.
  constexpr void Add(const T& value) {
    if (Count() == 0) {
      max_ = value;
      min_ = value;
    } else if (value > max_) {
      max_ = value;
    } else if (value < min_) {
      min_ = value;
    }

    data_.push_front(value);
    alpha_list_.push_front(alpha_);
  }

  int64_t Count() const { return data_.size(); }

  void Reset() {
    min_ = {};
    max_ = {};
    data_.clear();
    alpha_list_.clear();
  }

  D Weight() const {
    D weight{};
    D alpha_i(1.);
    for (size_t i = 0; i < data_.size(); ++i) {
      weight += alpha_i;
      alpha_i *= alpha_list_[i];
    }
    return weight;
  }

  D Weight2() const {
    D weight2{};
    D alpha2_i(1.);
    for (size_t i = 0; i < data_.size(); ++i) {
      weight2 += alpha2_i;
      alpha2_i *= alpha_list_[i] * alpha_list_[i];
    }
    return weight2;
  }

  D Mean() const {
    D wsum{};
    D alpha_i(1.);
    for (size_t i = 0; i < data_.size(); ++i) {
      wsum += alpha_i * data_[i];
      alpha_i *= alpha_list_[i];
    }
    return wsum / Weight();
  }

  // Should always return a positive value.
  D Variance() const {
    return GetUnweightedVariance() / (Weight() - Weight2() / Weight());
  }

  // Should always return a positive value.
  D PopulationVariance() const { return GetUnweightedVariance() / Weight(); }

  D StdDev() const { return std::sqrt(Variance()); }

  D PopulationStdDev() const { return std::sqrt(PopulationVariance()); }

  T Min() const { return min_; }

  T Max() const { return max_; }

  std::string ToString() const {
    const auto N = Count();
    if (N == 0) {
      return "unavail";
    }

    std::string result = "ave=" + std::to_string(Mean());
    if (N > 1) {
      result += " std=" + std::to_string(StdDev());
    }
    result += " min=" + std::to_string(Min());
    result += " max=" + std::to_string(Max());
    return result;
  }

 private:
  T min_{};
  T max_{};

  D alpha_;  // current alpha value
  std::deque<T>
      data_;  // store all the data for exact summation, data_[0] most recent.
  std::deque<D> alpha_list_;  // alpha value for the data added.

  D GetUnweightedVariance() const {
    const D mean = Mean();
    D wsum{};
    D alpha_i(1.);
    for (size_t i = 0; i < data_.size(); ++i) {
      const D diff = data_[i] - mean;
      wsum += alpha_i * diff * diff;
      alpha_i *= alpha_list_[i];
    }
    return wsum;
  }
};

// NOLINTEND(modernize-avoid-c-arrays)

}  // namespace base
}  // namespace ave

#endif  // AVE_STATISTICS_H
