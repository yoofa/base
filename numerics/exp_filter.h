/*
 * exp_filter.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef EXP_FILTER_H
#define EXP_FILTER_H

namespace yf {
namespace base {

// This class can be used, for example, for smoothing the result of bandwidth
// estimation and packet loss estimation.
class ExpFilter {
 public:
  static const float kValueUndefined;
  explicit ExpFilter(float alpha, float max = kValueUndefined);

  // Resets the filter to its initial state, and resets filter factor base to
  // the given value `alpha`.
  void Reset(float alpha);

  // Applies the filter with a given exponent on the provided sample:
  // y(k) = min(alpha_^ exp * y(k-1) + (1 - alpha_^ exp) * sample, max_).
  float Apply(float exp, float sample);

  // Returns current filtered value.
  float filtered() const { return filtered_; }

  // Changes the filter factor base to the given value `alpha`.
  void UpdateBase(float alpha);

 private:
  float alpha_;     // filter for factor base
  float filtered_;  // current filtered output
  const float max_;
};

}  // namespace base

}  // namespace yf

#endif /* !EXP_FILTER_H */
