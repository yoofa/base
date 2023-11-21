/*
 * exp_filter.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/exp_filter.h"

#include <cmath>

namespace yf {
namespace base {

const float ExpFilter::kValueUndefined = -1.0f;

ExpFilter::ExpFilter(float alpha, float max)
    : alpha_(alpha), filtered_(kValueUndefined), max_(max) {}

void ExpFilter::Reset(float alpha) {
  alpha_ = alpha;
  filtered_ = kValueUndefined;
}

float ExpFilter::Apply(float exp, float sample) {
  if (filtered_ == kValueUndefined) {
    // Initialize filtered value.
    filtered_ = sample;
  } else if (exp == 1.0) {
    filtered_ = alpha_ * filtered_ + (1 - alpha_) * sample;
  } else {
    float alpha = std::pow(alpha_, exp);
    filtered_ = alpha * filtered_ + (1 - alpha) * sample;
  }
  if (max_ != kValueUndefined && filtered_ > max_) {
    filtered_ = max_;
  }
  return filtered_;
}

void ExpFilter::UpdateBase(float alpha) {
  alpha_ = alpha;
}

}  // namespace base
}  // namespace yf
