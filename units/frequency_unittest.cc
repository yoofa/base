/*
 * frequency_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/units/frequency.h"

#include "gtest/gtest.h"

namespace ave {
namespace base {
namespace test {

TEST(FrequencyTest, ConstExpr) {
  constexpr int64_t kMilliHz = 30000;  // 30 Hz
  constexpr Frequency kZero = Frequency::MilliHertz(0);

  static_assert(kZero.IsZero());
  static_assert(!kZero.IsInfinite());

  constexpr Frequency kFreq = Frequency::MilliHertz(kMilliHz);
  static_assert(kFreq.millihertz() == kMilliHz);
  static_assert(kFreq.hertz() == 30);
}

TEST(FrequencyTest, GetBackSameValues) {
  EXPECT_EQ(Frequency::MilliHertz(1000).millihertz(), 1000);
  EXPECT_EQ(Frequency::Hertz(30).hertz(), 30);
  EXPECT_EQ(Frequency::KiloHertz(48).hertz(), 48000);
  EXPECT_EQ(Frequency::Zero().millihertz(), 0);
}

TEST(FrequencyTest, GetDifferentPrefix) {
  // 1 kHz = 1000 Hz = 1000000 mHz
  const Frequency f = Frequency::KiloHertz(1);
  EXPECT_EQ(f.hertz(), 1000);
  EXPECT_EQ(f.millihertz(), 1000000);

  // 48 kHz (audio sample rate)
  const Frequency audio = Frequency::KiloHertz(48);
  EXPECT_EQ(audio.hertz(), 48000);

  // 90 kHz (RTP clock)
  const Frequency rtp = Frequency::KiloHertz(90);
  EXPECT_EQ(rtp.hertz(), 90000);
}

TEST(FrequencyTest, IdentityChecks) {
  const Frequency f = Frequency::Hertz(60);

  EXPECT_TRUE(Frequency::Zero().IsZero());
  EXPECT_FALSE(f.IsZero());

  EXPECT_FALSE(f.IsInfinite());
  EXPECT_TRUE(f.IsFinite());
}

TEST(FrequencyTest, ComparisonOperators) {
  const Frequency slow = Frequency::Hertz(25);
  const Frequency fast = Frequency::Hertz(60);

  EXPECT_EQ(Frequency::Zero(), Frequency::MilliHertz(0));
  EXPECT_EQ(slow, Frequency::Hertz(25));
  EXPECT_NE(slow, fast);

  EXPECT_LT(slow, fast);
  EXPECT_LE(slow, fast);
  EXPECT_LE(slow, slow);
  EXPECT_GT(fast, slow);
  EXPECT_GE(fast, slow);
  EXPECT_GE(fast, fast);

  EXPECT_LT(Frequency::Zero(), slow);
}

TEST(FrequencyTest, Clamping) {
  const Frequency lower = Frequency::Hertz(15);
  const Frequency upper = Frequency::Hertz(60);
  const Frequency under = Frequency::Hertz(10);
  const Frequency inside = Frequency::Hertz(30);
  const Frequency over = Frequency::Hertz(120);

  EXPECT_EQ(under.Clamped(lower, upper), lower);
  EXPECT_EQ(inside.Clamped(lower, upper), inside);
  EXPECT_EQ(over.Clamped(lower, upper), upper);

  Frequency mutable_freq = under;
  mutable_freq.Clamp(lower, upper);
  EXPECT_EQ(mutable_freq, lower);

  mutable_freq = inside;
  mutable_freq.Clamp(lower, upper);
  EXPECT_EQ(mutable_freq, inside);

  mutable_freq = over;
  mutable_freq.Clamp(lower, upper);
  EXPECT_EQ(mutable_freq, upper);
}

TEST(FrequencyTest, MathOperations) {
  const Frequency fa = Frequency::Hertz(25);
  const Frequency fb = Frequency::Hertz(35);

  EXPECT_EQ((fa + fb).hertz(), 60);
  EXPECT_EQ((fb - fa).hertz(), 10);

  EXPECT_EQ((fb / 5).hertz(), 7);
  EXPECT_DOUBLE_EQ(fb / fa, static_cast<double>(35) / 25);

  Frequency mutable_f = fa;
  mutable_f += fb;
  EXPECT_EQ(mutable_f, Frequency::Hertz(60));
  mutable_f -= fb;
  EXPECT_EQ(mutable_f, fa);
}

TEST(FrequencyTest, MultiplyByScalar) {
  const Frequency kValue = Frequency::Hertz(30);
  const int64_t kInt64 = 2;
  const int32_t kInt32 = 3;
  const double kFloat = 1.5;

  EXPECT_EQ((kValue * kInt64).hertz(), 60);
  EXPECT_EQ(kValue * kInt64, kInt64 * kValue);

  EXPECT_EQ((kValue * kInt32).hertz(), 90);
  EXPECT_EQ(kValue * kInt32, kInt32 * kValue);

  EXPECT_DOUBLE_EQ((kValue * kFloat).millihertz(),
                   kValue.millihertz() * kFloat);
  EXPECT_EQ(kValue * kFloat, kFloat * kValue);
}

// --- Cross-type operator tests ---

TEST(FrequencyTest, IntDividedByTimeDelta) {
  // 1 / (33.333... ms) ≈ 30 Hz
  // Use exact value: 1 / (1/30 s) = 30 Hz
  // TimeDelta = 1000000/30 us = 33333 us (rounded)
  const TimeDelta period_30hz = TimeDelta::Micros(1000000 / 30);
  const Frequency f = 1 / period_30hz;
  // millihertz should be close to 30000
  EXPECT_NEAR(f.millihertz<double>(), 30000.0, 10.0);
}

TEST(FrequencyTest, IntDividedByFrequency) {
  // 1 / 30 Hz = period in seconds
  // 1 / 1000 Hz = 1 ms
  const Frequency f = Frequency::Hertz(1000);
  const TimeDelta period = 1 / f;
  EXPECT_EQ(period.us(), 1000);  // 1 ms = 1000 us
}

TEST(FrequencyTest, FrequencyMultipliedByTimeDelta) {
  // 30 Hz * (1/30 s) = 1.0
  const Frequency f = Frequency::Hertz(1);
  const TimeDelta t = TimeDelta::Seconds(5);
  EXPECT_DOUBLE_EQ(f * t, 5.0);
  EXPECT_DOUBLE_EQ(t * f, 5.0);
}

TEST(FrequencyTest, RoundTripMilliHertz) {
  // 90 kHz RTP clock: 90000 Hz = 90000000 mHz
  const Frequency rtp = Frequency::KiloHertz(90);
  EXPECT_EQ(rtp.hertz(), 90000);
  EXPECT_EQ(rtp.millihertz(), 90000000);
  // Period = 1/90000 s ≈ 11.11 us
  const TimeDelta period = 1 / rtp;
  EXPECT_NEAR(period.us<double>(), 1000000.0 / 90000.0, 1.0);
}

TEST(FrequencyTest, CommonVideoFrameRates) {
  EXPECT_EQ(Frequency::Hertz(24).hertz(), 24);
  EXPECT_EQ(Frequency::Hertz(25).hertz(), 25);
  EXPECT_EQ(Frequency::Hertz(30).hertz(), 30);
  EXPECT_EQ(Frequency::Hertz(60).hertz(), 60);
  EXPECT_EQ(Frequency::Hertz(120).hertz(), 120);
}

TEST(FrequencyTest, CommonAudioSampleRates) {
  EXPECT_EQ(Frequency::KiloHertz(8).hertz(), 8000);
  EXPECT_EQ(Frequency::KiloHertz(16).hertz(), 16000);
  EXPECT_EQ(Frequency::KiloHertz(44).hertz(), 44000);
  EXPECT_EQ(Frequency::KiloHertz(48).hertz(), 48000);
}

}  // namespace test
}  // namespace base
}  // namespace ave
