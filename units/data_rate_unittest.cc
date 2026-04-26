/*
 * data_rate_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/units/data_rate.h"

#include <limits>

#include "gtest/gtest.h"

namespace ave {
namespace base {
namespace test {

TEST(DataRateTest, ConstExpr) {
  constexpr int64_t kBps = 128000;
  constexpr DataRate kZero = DataRate::BitsPerSec(0);
  constexpr DataRate kInfinity = DataRate::Infinity();

  static_assert(kZero.IsZero());
  static_assert(kInfinity.IsPlusInfinity());
  static_assert(!kZero.IsInfinite());

  constexpr DataRate kRate = DataRate::BitsPerSec(kBps);
  static_assert(kRate.bps() == kBps);

  // fallback when infinite
  static_assert(kInfinity.bps_or(-1) == -1);
  static_assert(kRate.bps_or(-1) == kBps);
  static_assert(kInfinity.kbps_or(-1) == -1);
}

TEST(DataRateTest, GetBackSameValues) {
  const int64_t kBps = 64000;
  EXPECT_EQ(DataRate::BitsPerSec(kBps).bps(), kBps);
  EXPECT_EQ(DataRate::Zero().bps(), 0);

  // BytesPerSec: 1 byte/sec = 8 bits/sec
  EXPECT_EQ(DataRate::BytesPerSec(1000).bps(), 8000);
  EXPECT_EQ(DataRate::BytesPerSec(1000).bytes_per_sec(), 1000);

  // KilobitsPerSec: 1 kbps = 1000 bps
  EXPECT_EQ(DataRate::KilobitsPerSec(128).bps(), 128000);
  EXPECT_EQ(DataRate::KilobitsPerSec(128).kbps(), 128);
}

TEST(DataRateTest, GetDifferentPrefix) {
  // 8 Mbps = 8000 kbps = 8000000 bps = 1000000 bytes/sec
  const DataRate rate = DataRate::KilobitsPerSec(8000);
  EXPECT_EQ(rate.bps(), 8000000);
  EXPECT_EQ(rate.kbps(), 8000);
  EXPECT_EQ(rate.bytes_per_sec(), 1000000);
}

TEST(DataRateTest, IdentityChecks) {
  const int64_t kBps = 1000;

  EXPECT_TRUE(DataRate::Zero().IsZero());
  EXPECT_FALSE(DataRate::BitsPerSec(kBps).IsZero());

  EXPECT_TRUE(DataRate::Infinity().IsInfinite());
  EXPECT_TRUE(DataRate::Infinity().IsPlusInfinity());
  EXPECT_FALSE(DataRate::Zero().IsInfinite());
  EXPECT_FALSE(DataRate::BitsPerSec(kBps).IsInfinite());

  EXPECT_FALSE(DataRate::Infinity().IsFinite());
  EXPECT_TRUE(DataRate::Zero().IsFinite());
  EXPECT_TRUE(DataRate::BitsPerSec(kBps).IsFinite());
}

TEST(DataRateTest, ComparisonOperators) {
  const DataRate slow = DataRate::KilobitsPerSec(100);
  const DataRate fast = DataRate::KilobitsPerSec(200);

  EXPECT_EQ(DataRate::Zero(), DataRate::BitsPerSec(0));
  EXPECT_EQ(slow, DataRate::KilobitsPerSec(100));
  EXPECT_NE(slow, fast);

  EXPECT_LT(slow, fast);
  EXPECT_LE(slow, fast);
  EXPECT_LE(slow, slow);
  EXPECT_GT(fast, slow);
  EXPECT_GE(fast, slow);
  EXPECT_GE(fast, fast);

  EXPECT_LT(DataRate::Zero(), slow);
  EXPECT_GT(DataRate::Infinity(), fast);
}

TEST(DataRateTest, Clamping) {
  const DataRate lower = DataRate::KilobitsPerSec(100);
  const DataRate upper = DataRate::KilobitsPerSec(500);
  const DataRate under = DataRate::KilobitsPerSec(50);
  const DataRate inside = DataRate::KilobitsPerSec(300);
  const DataRate over = DataRate::KilobitsPerSec(800);

  EXPECT_EQ(under.Clamped(lower, upper), lower);
  EXPECT_EQ(inside.Clamped(lower, upper), inside);
  EXPECT_EQ(over.Clamped(lower, upper), upper);

  DataRate mutable_rate = under;
  mutable_rate.Clamp(lower, upper);
  EXPECT_EQ(mutable_rate, lower);

  mutable_rate = inside;
  mutable_rate.Clamp(lower, upper);
  EXPECT_EQ(mutable_rate, inside);

  mutable_rate = over;
  mutable_rate.Clamp(lower, upper);
  EXPECT_EQ(mutable_rate, upper);
}

TEST(DataRateTest, MathOperations) {
  const int64_t kBpsA = 200000;
  const int64_t kBpsB = 800000;
  const DataRate rate_a = DataRate::BitsPerSec(kBpsA);
  const DataRate rate_b = DataRate::BitsPerSec(kBpsB);

  EXPECT_EQ((rate_a + rate_b).bps(), kBpsA + kBpsB);
  EXPECT_EQ((rate_b - rate_a).bps(), kBpsB - kBpsA);

  EXPECT_EQ((rate_b / 4).bps(), kBpsB / 4);
  EXPECT_DOUBLE_EQ(rate_b / rate_a, static_cast<double>(kBpsB) / kBpsA);

  DataRate mutable_rate = rate_a;
  mutable_rate += rate_b;
  EXPECT_EQ(mutable_rate, DataRate::BitsPerSec(kBpsA + kBpsB));
  mutable_rate -= rate_b;
  EXPECT_EQ(mutable_rate, rate_a);
}

TEST(DataRateTest, MultiplyByScalar) {
  const DataRate kValue = DataRate::KilobitsPerSec(100);
  const int64_t kInt64 = 3;
  const int32_t kInt32 = 5;
  const double kFloat = 1.5;

  EXPECT_EQ((kValue * kInt64).bps(), kValue.bps() * kInt64);
  EXPECT_EQ(kValue * kInt64, kInt64 * kValue);

  EXPECT_EQ((kValue * kInt32).bps(), kValue.bps() * kInt32);
  EXPECT_EQ(kValue * kInt32, kInt32 * kValue);

  EXPECT_DOUBLE_EQ((kValue * kFloat).bps(), kValue.bps() * kFloat);
  EXPECT_EQ(kValue * kFloat, kFloat * kValue);
}

TEST(DataRateTest, InfinityOperations) {
  const DataRate finite = DataRate::KilobitsPerSec(1000);
  EXPECT_TRUE((DataRate::Infinity() + finite).IsPlusInfinity());
  EXPECT_TRUE((finite + DataRate::Infinity()).IsPlusInfinity());
}

// --- Cross-type operator tests ---

TEST(DataRateTest, DataSizeDividedByTimeDelta) {
  // 8000 bytes / 1 second = 64 kbps
  const DataSize size = DataSize::Bytes(8000);
  const TimeDelta duration = TimeDelta::Seconds(1);
  const DataRate rate = size / duration;
  EXPECT_EQ(rate.bps(), 64000);
}

TEST(DataRateTest, DataSizeDividedByDataRate) {
  // 8000 bytes / 64000 bps = 1 second
  const DataSize size = DataSize::Bytes(8000);
  const DataRate rate = DataRate::BitsPerSec(64000);
  const TimeDelta duration = size / rate;
  EXPECT_EQ(duration.seconds(), 1);
}

TEST(DataRateTest, DataRateMultipliedByTimeDelta) {
  // 64000 bps * 1 s = 8000 bytes
  const DataRate rate = DataRate::BitsPerSec(64000);
  const TimeDelta duration = TimeDelta::Seconds(1);
  EXPECT_EQ((rate * duration).bytes(), 8000);
  EXPECT_EQ((duration * rate).bytes(), 8000);
}

TEST(DataRateTest, RoundTripConversions) {
  // bps -> bytes_per_sec -> bps (only exact when divisible by 8)
  const DataRate rate = DataRate::BytesPerSec(125000);  // 1 Mbps
  EXPECT_EQ(rate.bps(), 1000000);
  EXPECT_EQ(rate.kbps(), 1000);
  EXPECT_EQ(rate.bytes_per_sec(), 125000);
}

TEST(DataRateTest, ConstructFromDouble) {
  const double kKbps = 1500.0;
  EXPECT_EQ(DataRate::KilobitsPerSec(kKbps).kbps(), 1500);

  const double kPlusInf = std::numeric_limits<double>::infinity();
  EXPECT_TRUE(DataRate::BitsPerSec(kPlusInf).IsPlusInfinity());
}

}  // namespace test
}  // namespace base
}  // namespace ave
