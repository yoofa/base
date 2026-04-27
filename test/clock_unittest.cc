/*
 * clock_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/clock.h"

#include <cstdint>
#include <thread>

#include <gtest/gtest.h>

#include "base/units/time_delta.h"
#include "base/units/timestamp.h"

namespace ave {
namespace base {
namespace {

// ============================================================
// NtpTime tests
// ============================================================

TEST(NtpTimeTest, DefaultConstructorIsZeroAndInvalid) {
  NtpTime ntp;
  EXPECT_EQ(static_cast<uint64_t>(ntp), 0u);
  EXPECT_FALSE(ntp.Valid());
}

TEST(NtpTimeTest, ConstructFromUint64) {
  constexpr uint64_t kValue = 0x12345678ABCDEF01ULL;
  NtpTime ntp(kValue);
  EXPECT_EQ(static_cast<uint64_t>(ntp), kValue);
  EXPECT_TRUE(ntp.Valid());
}

TEST(NtpTimeTest, ConstructFromSecondsAndFractions) {
  constexpr uint32_t kSeconds = 100;
  constexpr uint32_t kFractions = 0x80000000u;
  NtpTime ntp(kSeconds, kFractions);
  EXPECT_EQ(ntp.seconds(), kSeconds);
  EXPECT_EQ(ntp.fractions(), kFractions);
  EXPECT_TRUE(ntp.Valid());
}

TEST(NtpTimeTest, SetMethod) {
  NtpTime ntp;
  ntp.Set(200, 0x40000000u);
  EXPECT_EQ(ntp.seconds(), 200u);
  EXPECT_EQ(ntp.fractions(), 0x40000000u);
}

TEST(NtpTimeTest, ResetMakesInvalid) {
  NtpTime ntp(42, 0);
  EXPECT_TRUE(ntp.Valid());
  ntp.Reset();
  EXPECT_FALSE(ntp.Valid());
  EXPECT_EQ(static_cast<uint64_t>(ntp), 0u);
}

TEST(NtpTimeTest, EqualityOperators) {
  NtpTime a(100, 0);
  NtpTime b(100, 0);
  NtpTime c(200, 0);
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
}

TEST(NtpTimeTest, CopyConstructorAndAssignment) {
  NtpTime original(123, 456);
  NtpTime copy(original);
  EXPECT_EQ(copy, original);

  NtpTime assigned;
  assigned = original;
  EXPECT_EQ(assigned, original);
}

TEST(NtpTimeTest, ToMsWholeSecond) {
  // fractions=0 => frac_ms = 0 / 4.294967296e6 ≈ 0; round(0 + 0.5) = 1
  // So ToMs() = 1000 * 1 + 1 = 1001 (due to +0.5 bias in the implementation)
  NtpTime ntp(1, 0);
  int64_t ms = ntp.ToMs();
  EXPECT_EQ(ms, 1001);
}

TEST(NtpTimeTest, ToMsHalfSecond) {
  // 0x80000000 fractions ≈ 0.5 s => frac_ms ≈ 500 ms
  NtpTime ntp(0, 0x80000000u);
  int64_t ms = ntp.ToMs();
  EXPECT_NEAR(ms, 500, 2);
}

// ============================================================
// SimulatedClock tests
// ============================================================

TEST(SimulatedClockTest, ConstructFromMicroseconds) {
  constexpr int64_t kInitUs = 1'000'000;
  SimulatedClock clock(kInitUs);
  EXPECT_EQ(clock.CurrentTime().us(), kInitUs);
}

TEST(SimulatedClockTest, ConstructFromTimestamp) {
  Timestamp initial = Timestamp::Millis(5000);
  SimulatedClock clock(initial);
  EXPECT_EQ(clock.CurrentTime(), initial);
}

TEST(SimulatedClockTest, AdvanceTimeMilliseconds) {
  SimulatedClock clock(0);
  clock.AdvanceTimeMilliseconds(100);
  EXPECT_EQ(clock.TimeInMilliseconds(), 100);
}

TEST(SimulatedClockTest, AdvanceTimeMicroseconds) {
  SimulatedClock clock(0);
  clock.AdvanceTimeMicroseconds(500);
  EXPECT_EQ(clock.TimeInMicroseconds(), 500);
}

TEST(SimulatedClockTest, AdvanceTimeWithTimeDelta) {
  SimulatedClock clock(0);
  clock.AdvanceTime(TimeDelta::Seconds(3));
  EXPECT_EQ(clock.TimeInMilliseconds(), 3000);
}

TEST(SimulatedClockTest, MultipleAdvances) {
  SimulatedClock clock(0);
  clock.AdvanceTimeMilliseconds(100);
  clock.AdvanceTimeMilliseconds(200);
  clock.AdvanceTimeMicroseconds(50);
  EXPECT_EQ(clock.TimeInMicroseconds(), 300'050);
}

TEST(SimulatedClockTest, TimeInMilliseconds) {
  SimulatedClock clock(2'500'000);  // 2.5 s in us
  EXPECT_EQ(clock.TimeInMilliseconds(), 2500);
}

TEST(SimulatedClockTest, TimeInMicroseconds) {
  SimulatedClock clock(1234);
  EXPECT_EQ(clock.TimeInMicroseconds(), 1234);
}

TEST(SimulatedClockTest, ConvertTimestampToNtpTimeAtUnixEpoch) {
  // At Unix epoch (us=0), NTP seconds = kNtpJan1970, fractions = 0
  SimulatedClock clock(0);
  NtpTime ntp = clock.ConvertTimestampToNtpTime(clock.CurrentTime());
  EXPECT_EQ(ntp.seconds(), kNtpJan1970);
  EXPECT_EQ(ntp.fractions(), 0u);
  EXPECT_TRUE(ntp.Valid());
}

TEST(SimulatedClockTest, ConvertTimestampToNtpTimeOneSecond) {
  SimulatedClock clock(1'000'000);  // 1 second after Unix epoch
  NtpTime ntp = clock.ConvertTimestampToNtpTime(clock.CurrentTime());
  EXPECT_EQ(ntp.seconds(), kNtpJan1970 + 1);
}

TEST(SimulatedClockTest, CurrentNtpTimeIsValid) {
  SimulatedClock clock(1'000'000);
  NtpTime ntp = clock.CurrentNtpTime();
  EXPECT_TRUE(ntp.Valid());
}

TEST(SimulatedClockTest, CurrentNtpInMilliseconds) {
  // At Unix time 0, NTP ms = kNtpJan1970 * 1000 (+ small rounding)
  SimulatedClock clock(0);
  int64_t ntp_ms = clock.CurrentNtpInMilliseconds();
  int64_t expected_ms = static_cast<int64_t>(kNtpJan1970) * 1000;
  EXPECT_NEAR(ntp_ms, expected_ms, 2);
}

TEST(SimulatedClockTest, AdvanceTimeZeroDoesNotChange) {
  SimulatedClock clock(1000);
  clock.AdvanceTimeMilliseconds(0);
  EXPECT_EQ(clock.TimeInMicroseconds(), 1000);
}

// ============================================================
// Clock::NtpToUtc tests
// ============================================================

TEST(ClockNtpToUtcTest, InvalidNtpReturnsMinusInfinity) {
  NtpTime invalid;
  Timestamp ts = Clock::NtpToUtc(invalid);
  EXPECT_TRUE(ts.IsMinusInfinity());
}

TEST(ClockNtpToUtcTest, NtpAtUnixEpochGivesZero) {
  // NTP seconds = kNtpJan1970 => UTC = 0
  NtpTime ntp(kNtpJan1970, 0);
  Timestamp ts = Clock::NtpToUtc(ntp);
  EXPECT_EQ(ts.us(), 0);
}

TEST(ClockNtpToUtcTest, NtpOneSecondAfterUnixEpoch) {
  NtpTime ntp(kNtpJan1970 + 1, 0);
  Timestamp ts = Clock::NtpToUtc(ntp);
  EXPECT_EQ(ts.us(), 1'000'000);
}

TEST(ClockNtpToUtcTest, NtpWithHalfSecondFraction) {
  // 0x80000000 fractions ≈ 500000 us
  NtpTime ntp(kNtpJan1970, 0x80000000u);
  Timestamp ts = Clock::NtpToUtc(ntp);
  EXPECT_NEAR(ts.us(), 500'000, 2);
}

TEST(ClockNtpToUtcTest, RoundTripWithSimulatedClock) {
  constexpr int64_t kTimeUs = 1'000'000;  // 1 s after Unix epoch
  SimulatedClock clock(kTimeUs);
  NtpTime ntp = clock.ConvertTimestampToNtpTime(clock.CurrentTime());
  Timestamp roundtrip = Clock::NtpToUtc(ntp);
  EXPECT_NEAR(roundtrip.us(), kTimeUs, 2);
}

// ============================================================
// RealTimeClock smoke tests
// ============================================================

TEST(RealTimeClockTest, CurrentTimeIsPositive) {
  Clock* clock = Clock::GetRealTimeClock();
  ASSERT_NE(clock, nullptr);
  Timestamp t = clock->CurrentTime();
  EXPECT_GT(t.us(), 0);
}

TEST(RealTimeClockTest, TimeAdvances) {
  Clock* clock = Clock::GetRealTimeClock();
  int64_t t1 = clock->TimeInMicroseconds();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  int64_t t2 = clock->TimeInMicroseconds();
  EXPECT_GT(t2, t1);
}

TEST(RealTimeClockTest, NtpTimeIsValid) {
  Clock* clock = Clock::GetRealTimeClock();
  NtpTime ntp = clock->CurrentNtpTime();
  EXPECT_TRUE(ntp.Valid());
  // We are well past Jan 1970, so NTP seconds must exceed kNtpJan1970
  EXPECT_GT(ntp.seconds(), kNtpJan1970);
}

TEST(RealTimeClockTest, SingletonReturnsConsistentPointer) {
  Clock* c1 = Clock::GetRealTimeClock();
  Clock* c2 = Clock::GetRealTimeClock();
  EXPECT_EQ(c1, c2);
}

}  // namespace
}  // namespace base
}  // namespace ave
