/*
 * data_size_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/units/data_size.h"

#include <limits>

#include "gtest/gtest.h"

namespace ave {
namespace base {
namespace test {

TEST(DataSizeTest, ConstExpr) {
  constexpr int64_t kValue = 12345;
  constexpr DataSize kZero = DataSize::Zero();
  constexpr DataSize kInfinity = DataSize::Infinity();

  static_assert(kZero.IsZero());
  static_assert(kInfinity.IsPlusInfinity());
  static_assert(!kZero.IsInfinite());

  constexpr DataSize kBytes = DataSize::Bytes(kValue);
  static_assert(kBytes.bytes() == kValue);

  // bytes_or returns fallback when infinite
  static_assert(kInfinity.bytes_or(-1) == -1);
  static_assert(kBytes.bytes_or(-1) == kValue);
}

TEST(DataSizeTest, GetBackSameValues) {
  const int64_t kValue = 12345;
  EXPECT_EQ(DataSize::Bytes(kValue).bytes(), kValue);
  EXPECT_EQ(DataSize::Zero().bytes(), 0);
}

TEST(DataSizeTest, IdentityChecks) {
  const int64_t kValue = 100;

  EXPECT_TRUE(DataSize::Zero().IsZero());
  EXPECT_FALSE(DataSize::Bytes(kValue).IsZero());

  EXPECT_TRUE(DataSize::Infinity().IsInfinite());
  EXPECT_TRUE(DataSize::Infinity().IsPlusInfinity());
  EXPECT_FALSE(DataSize::Zero().IsInfinite());
  EXPECT_FALSE(DataSize::Bytes(kValue).IsInfinite());

  EXPECT_FALSE(DataSize::Infinity().IsFinite());
  EXPECT_TRUE(DataSize::Zero().IsFinite());
  EXPECT_TRUE(DataSize::Bytes(kValue).IsFinite());
}

TEST(DataSizeTest, ComparisonOperators) {
  const DataSize small = DataSize::Bytes(100);
  const DataSize large = DataSize::Bytes(200);

  EXPECT_EQ(DataSize::Zero(), DataSize::Bytes(0));
  EXPECT_EQ(small, DataSize::Bytes(100));
  EXPECT_NE(small, large);

  EXPECT_LT(small, large);
  EXPECT_LE(small, large);
  EXPECT_LE(small, small);
  EXPECT_GT(large, small);
  EXPECT_GE(large, small);
  EXPECT_GE(large, large);

  EXPECT_LT(DataSize::Zero(), small);
  EXPECT_GT(DataSize::Infinity(), large);
}

TEST(DataSizeTest, Clamping) {
  const DataSize lower = DataSize::Bytes(100);
  const DataSize upper = DataSize::Bytes(500);
  const DataSize under = DataSize::Bytes(50);
  const DataSize inside = DataSize::Bytes(300);
  const DataSize over = DataSize::Bytes(800);

  EXPECT_EQ(under.Clamped(lower, upper), lower);
  EXPECT_EQ(inside.Clamped(lower, upper), inside);
  EXPECT_EQ(over.Clamped(lower, upper), upper);

  DataSize mutable_size = under;
  mutable_size.Clamp(lower, upper);
  EXPECT_EQ(mutable_size, lower);

  mutable_size = inside;
  mutable_size.Clamp(lower, upper);
  EXPECT_EQ(mutable_size, inside);

  mutable_size = over;
  mutable_size.Clamp(lower, upper);
  EXPECT_EQ(mutable_size, upper);
}

TEST(DataSizeTest, MathOperations) {
  const int64_t kValueA = 300;
  const int64_t kValueB = 700;
  const DataSize size_a = DataSize::Bytes(kValueA);
  const DataSize size_b = DataSize::Bytes(kValueB);

  EXPECT_EQ((size_a + size_b).bytes(), kValueA + kValueB);
  EXPECT_EQ((size_b - size_a).bytes(), kValueB - kValueA);

  EXPECT_EQ((size_b / 7).bytes(), kValueB / 7);
  EXPECT_DOUBLE_EQ(size_b / size_a, static_cast<double>(kValueB) / kValueA);

  DataSize mutable_size = size_a;
  mutable_size += size_b;
  EXPECT_EQ(mutable_size, DataSize::Bytes(kValueA + kValueB));
  mutable_size -= size_b;
  EXPECT_EQ(mutable_size, size_a);
}

TEST(DataSizeTest, MultiplyByScalar) {
  const DataSize kValue = DataSize::Bytes(100);
  const int64_t kInt64 = 3;
  const int32_t kInt32 = 5;
  const double kFloat = 2.5;

  EXPECT_EQ((kValue * kInt64).bytes(), kValue.bytes() * kInt64);
  EXPECT_EQ(kValue * kInt64, kInt64 * kValue);

  EXPECT_EQ((kValue * kInt32).bytes(), kValue.bytes() * kInt32);
  EXPECT_EQ(kValue * kInt32, kInt32 * kValue);

  EXPECT_EQ((kValue * kFloat).bytes(),
            static_cast<int64_t>(kValue.bytes() * kFloat));
  EXPECT_EQ(kValue * kFloat, kFloat * kValue);
}

TEST(DataSizeTest, ConstructFromLargeInt) {
  const int kMaxInt = std::numeric_limits<int>::max();
  EXPECT_EQ(DataSize::Bytes(kMaxInt).bytes(), static_cast<int64_t>(kMaxInt));
}

TEST(DataSizeTest, InfinityOperations) {
  const DataSize finite = DataSize::Bytes(500);
  EXPECT_TRUE((DataSize::Infinity() + finite).IsPlusInfinity());
  EXPECT_TRUE((finite + DataSize::Infinity()).IsPlusInfinity());
}

}  // namespace test
}  // namespace base
}  // namespace ave
