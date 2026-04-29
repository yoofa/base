/*
 * safe_compare_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/safe_compare.h"

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

// ============================================================
// SafeEq / SafeNe
// ============================================================

TEST(SafeCompareTest, EqSameSigned) {
  EXPECT_TRUE(SafeEq(5, 5));
  EXPECT_FALSE(SafeEq(5, 6));
}

TEST(SafeCompareTest, EqSignedUnsigned) {
  // -1 should never equal any unsigned value
  EXPECT_FALSE(SafeEq(-1, 1u));
  EXPECT_TRUE(SafeEq(1, uint32_t{1} + 0));
  EXPECT_TRUE(SafeEq(5, 5u));
}

TEST(SafeCompareTest, EqUnsignedLargeValue) {
  // std::numeric_limits<int32_t>::max() + 1 as uint32_t vs int32_t
  uint32_t big = static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) + 1;
  EXPECT_FALSE(SafeEq(big, std::numeric_limits<int32_t>::max()));
  EXPECT_FALSE(SafeEq(std::numeric_limits<int32_t>::max(), big));
}

TEST(SafeCompareTest, NeBasic) {
  EXPECT_TRUE(SafeNe(5, 6));
  EXPECT_FALSE(SafeNe(5, 5));
  EXPECT_TRUE(SafeNe(-1, 1u));
}

// ============================================================
// SafeLt / SafeLe / SafeGt / SafeGe
// ============================================================

TEST(SafeCompareTest, LtSameSigned) {
  EXPECT_TRUE(SafeLt(3, 5));
  EXPECT_FALSE(SafeLt(5, 3));
  EXPECT_FALSE(SafeLt(5, 5));
}

TEST(SafeCompareTest, LtSignedNegativeVsUnsigned) {
  // -1 < any unsigned value
  EXPECT_TRUE(SafeLt(-1, 0u));
  EXPECT_TRUE(SafeLt(-1, 1u));
  EXPECT_TRUE(SafeLt(int32_t{-1}, uint32_t{0}));
}

TEST(SafeCompareTest, LtUnsignedVsSigned) {
  EXPECT_FALSE(SafeLt(0u, -1));
  EXPECT_FALSE(SafeLt(1u, -1));
}

TEST(SafeCompareTest, LeBasic) {
  EXPECT_TRUE(SafeLe(5, 5));
  EXPECT_TRUE(SafeLe(4, 5));
  EXPECT_FALSE(SafeLe(6, 5));
}

TEST(SafeCompareTest, LeSignedUnsigned) {
  EXPECT_TRUE(SafeLe(-1, 0u));
  EXPECT_FALSE(SafeLe(1u, -1));
}

TEST(SafeCompareTest, GtBasic) {
  EXPECT_TRUE(SafeGt(5, 4));
  EXPECT_FALSE(SafeGt(4, 5));
  EXPECT_FALSE(SafeGt(5, 5));
}

TEST(SafeCompareTest, GtSignedUnsigned) {
  // No unsigned value is > -1
  EXPECT_FALSE(SafeGt(-1, 0u));
  // Any unsigned is > negative
  EXPECT_TRUE(SafeGt(0u, -1));
}

TEST(SafeCompareTest, GeBasic) {
  EXPECT_TRUE(SafeGe(5, 5));
  EXPECT_TRUE(SafeGe(6, 5));
  EXPECT_FALSE(SafeGe(4, 5));
}

TEST(SafeCompareTest, GeSignedUnsigned) {
  EXPECT_FALSE(SafeGe(-1, 0u));
  EXPECT_TRUE(SafeGe(0u, -1));
}

// ============================================================
// Cross-type edge cases
// ============================================================

TEST(SafeCompareTest, Int8VsUint64) {
  int8_t neg = -1;
  uint64_t big = std::numeric_limits<uint64_t>::max();
  EXPECT_TRUE(SafeLt(neg, big));
  EXPECT_FALSE(SafeGt(neg, big));
  EXPECT_FALSE(SafeEq(neg, big));
}

TEST(SafeCompareTest, Uint8VsInt64Max) {
  uint8_t u = 255;
  int64_t s = std::numeric_limits<int64_t>::max();
  EXPECT_TRUE(SafeLt(u, s));
  EXPECT_TRUE(SafeGt(s, u));
}

}  // namespace
}  // namespace base
}  // namespace ave
