/*
 * safe_conversions_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/safe_conversions.h"

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

// ============================================================
// IsValueInRangeForNumericType tests
// ============================================================

TEST(IsValueInRangeTest, Int32InInt64) {
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(int64_t{0}));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(
      int64_t{std::numeric_limits<int32_t>::max()}));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(
      int64_t{std::numeric_limits<int32_t>::min()}));
}

TEST(IsValueInRangeTest, Int64OverflowsInt32) {
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(
      int64_t{std::numeric_limits<int32_t>::max()} + 1));
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(
      int64_t{std::numeric_limits<int32_t>::min()} - 1));
}

TEST(IsValueInRangeTest, NegativeInUnsigned) {
  EXPECT_FALSE(IsValueInRangeForNumericType<uint32_t>(-1));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint8_t>(int32_t{-1}));
}

TEST(IsValueInRangeTest, PositiveInUnsigned) {
  EXPECT_TRUE(IsValueInRangeForNumericType<uint8_t>(int32_t{0}));
  EXPECT_TRUE(IsValueInRangeForNumericType<uint8_t>(int32_t{255}));
  EXPECT_FALSE(IsValueInRangeForNumericType<uint8_t>(int32_t{256}));
}

TEST(IsValueInRangeTest, FloatToInt) {
  // Range check only verifies bounds, not integer exactness
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(1.0f));
  EXPECT_TRUE(IsValueInRangeForNumericType<int32_t>(
      1.5f));  // in range (not exact, but in bounds)
  // Out-of-range float
  EXPECT_FALSE(IsValueInRangeForNumericType<int32_t>(5e10f));
}

// ============================================================
// saturated_cast tests
// ============================================================

TEST(SaturatedCastTest, InRange) {
  EXPECT_EQ(saturated_cast<int32_t>(int64_t{100}), 100);
  EXPECT_EQ(saturated_cast<uint8_t>(int32_t{200}), 200);
}

TEST(SaturatedCastTest, SaturatesAtMax) {
  int64_t big = int64_t{std::numeric_limits<int32_t>::max()} + 100;
  EXPECT_EQ(saturated_cast<int32_t>(big), std::numeric_limits<int32_t>::max());
  EXPECT_EQ(saturated_cast<uint8_t>(int32_t{500}),
            std::numeric_limits<uint8_t>::max());
}

TEST(SaturatedCastTest, SaturatesAtMin) {
  int64_t small = int64_t{std::numeric_limits<int32_t>::min()} - 100;
  EXPECT_EQ(saturated_cast<int32_t>(small),
            std::numeric_limits<int32_t>::min());
  EXPECT_EQ(saturated_cast<int8_t>(int32_t{-200}),
            std::numeric_limits<int8_t>::min());
}

TEST(SaturatedCastTest, NegativeToUnsignedSaturatesAtZero) {
  EXPECT_EQ(saturated_cast<uint8_t>(-1), 0);
  EXPECT_EQ(saturated_cast<uint32_t>(-100), 0u);
}

TEST(SaturatedCastTest, FloatToInt) {
  EXPECT_EQ(saturated_cast<int32_t>(42.9f), 42);
  EXPECT_EQ(saturated_cast<int32_t>(-42.9f), -42);
}

TEST(SaturatedCastTest, LargeFloatSaturates) {
  float too_big =
      static_cast<float>(std::numeric_limits<int32_t>::max()) * 2.0f;
  EXPECT_EQ(saturated_cast<int32_t>(too_big),
            std::numeric_limits<int32_t>::max());
}

// ============================================================
// checked_cast tests (should not CHECK for valid values)
// ============================================================

TEST(CheckedCastTest, InRangeSuccess) {
  EXPECT_EQ(checked_cast<int32_t>(int64_t{42}), 42);
  EXPECT_EQ(checked_cast<uint8_t>(int32_t{255}), 255);
  EXPECT_EQ(checked_cast<uint8_t>(int32_t{0}), 0);
}

}  // namespace
}  // namespace base
}  // namespace ave
