/*
 * divide_round_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/divide_round.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

// ============================================================
// DivideRoundUp tests
// ============================================================

TEST(DivideRoundUpTest, ExactDivision) {
  EXPECT_EQ(DivideRoundUp(10, 2), 5);
  EXPECT_EQ(DivideRoundUp(9, 3), 3);
  EXPECT_EQ(DivideRoundUp(0, 5), 0);
}

TEST(DivideRoundUpTest, WithRemainder) {
  EXPECT_EQ(DivideRoundUp(7, 2), 4);   // 3.5 -> 4
  EXPECT_EQ(DivideRoundUp(1, 3), 1);   // 0.33 -> 1
  EXPECT_EQ(DivideRoundUp(11, 4), 3);  // 2.75 -> 3
}

TEST(DivideRoundUpTest, DivisorLargerThanDividend) {
  EXPECT_EQ(DivideRoundUp(1, 100), 1);
  EXPECT_EQ(DivideRoundUp(0, 100), 0);
}

TEST(DivideRoundUpTest, DivisorOne) {
  EXPECT_EQ(DivideRoundUp(7, 1), 7);
  EXPECT_EQ(DivideRoundUp(0, 1), 0);
}

TEST(DivideRoundUpTest, LargeValues) {
  EXPECT_EQ(DivideRoundUp(1000000, 3), 333334);
  EXPECT_EQ(DivideRoundUp(1000000, 1000000), 1);
  EXPECT_EQ(DivideRoundUp(1000001, 1000000), 2);
}

TEST(DivideRoundUpTest, MixedIntegerTypes) {
  // int and int64_t mix
  EXPECT_EQ(DivideRoundUp(7, 2), 4);
  EXPECT_EQ(DivideRoundUp(int64_t{7}, int64_t{2}), 4);
}

// ============================================================
// DivideRoundToNearest tests
// ============================================================

TEST(DivideRoundToNearestTest, ExactDivision) {
  EXPECT_EQ(DivideRoundToNearest(10, 2), 5);
  EXPECT_EQ(DivideRoundToNearest(9, 3), 3);
  EXPECT_EQ(DivideRoundToNearest(0, 5), 0);
}

TEST(DivideRoundToNearestTest, RoundDown) {
  EXPECT_EQ(DivideRoundToNearest(7, 4), 2);  // 1.75 -> 2
  EXPECT_EQ(DivideRoundToNearest(3, 4), 1);  // 0.75 -> 1
  EXPECT_EQ(DivideRoundToNearest(1, 4), 0);  // 0.25 -> 0
}

TEST(DivideRoundToNearestTest, RoundUp) {
  EXPECT_EQ(DivideRoundToNearest(3, 2), 2);  // 1.5 -> 2 (round half up)
  EXPECT_EQ(DivideRoundToNearest(5, 2), 3);  // 2.5 -> 3
}

TEST(DivideRoundToNearestTest, NegativeDividend) {
  EXPECT_EQ(DivideRoundToNearest(-7, 4), -2);  // -1.75 -> -2
  EXPECT_EQ(DivideRoundToNearest(-3, 4), -1);  // -0.75 -> -1
  EXPECT_EQ(DivideRoundToNearest(-1, 4), 0);   // -0.25 -> 0
}

TEST(DivideRoundToNearestTest, NegativeDividendHalfway) {
  EXPECT_EQ(DivideRoundToNearest(-3, 2), -1);  // -1.5 -> -1 (toward zero)
  EXPECT_EQ(DivideRoundToNearest(-5, 2), -2);  // -2.5 -> -2
}

TEST(DivideRoundToNearestTest, DivisorOne) {
  EXPECT_EQ(DivideRoundToNearest(7, 1), 7);
  EXPECT_EQ(DivideRoundToNearest(-7, 1), -7);
  EXPECT_EQ(DivideRoundToNearest(0, 1), 0);
}

}  // namespace
}  // namespace base
}  // namespace ave
