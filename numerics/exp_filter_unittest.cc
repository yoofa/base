/*
 * exp_filter_unittest.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/exp_filter.h"

#include "test/gtest.h"

namespace ave {
namespace base {
TEST(ExpFilterTest, FirstTimeOutputEqualInput) {
  // No max value defined.
  ExpFilter filter = ExpFilter(0.9f);
  filter.Apply(100.0f, 10.0f);

  // First time, first argument no effect.
  double value = 10.0f;
  EXPECT_FLOAT_EQ(value, filter.filtered());
}

TEST(ExpFilterTest, SecondTime) {
  float value;

  ExpFilter filter = ExpFilter(0.9f);
  filter.Apply(100.0f, 10.0f);

  // First time, first argument no effect.
  value = 10.0f;

  filter.Apply(10.0f, 20.0f);
  float alpha = std::pow(0.9f, 10.0f);
  value = alpha * value + (1.0f - alpha) * 20.0f;
  EXPECT_FLOAT_EQ(value, filter.filtered());
}

TEST(ExpFilterTest, Reset) {
  ExpFilter filter = ExpFilter(0.9f);
  filter.Apply(100.0f, 10.0f);

  filter.Reset(0.8f);
  filter.Apply(100.0f, 1.0f);

  // Become first time after a reset.
  double value = 1.0f;
  EXPECT_FLOAT_EQ(value, filter.filtered());
}

TEST(ExpfilterTest, OutputLimitedByMax) {
  double value;

  // Max value defined.
  ExpFilter filter = ExpFilter(0.9f, 1.0f);
  filter.Apply(100.0f, 10.0f);

  // Limited to max value.
  value = 1.0f;
  EXPECT_EQ(value, filter.filtered());

  filter.Apply(1.0f, 0.0f);
  value = 0.9f * value;
}

}  // namespace base
}  // namespace ave
