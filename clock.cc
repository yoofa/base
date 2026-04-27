/*
 * clock.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/clock.h"

#include "base/checks.h"
#include "base/numerics/divide_round.h"
#include "base/time_utils.h"

namespace ave {
namespace base {

namespace {

int64_t NtpOffsetUsCalledOnce() {
  constexpr int64_t kNtpJan1970Sec = 2208988800;
  int64_t clock_time = TimeMicros();
  int64_t utc_time = TimeUTCMicros();
  return utc_time - clock_time + kNtpJan1970Sec * kNumMicrosecsPerSec;
}

NtpTime TimeMicrosToNtp(int64_t time_us) {
  static int64_t ntp_offset_us = NtpOffsetUsCalledOnce();

  int64_t time_ntp_us = time_us + ntp_offset_us;
  AVE_DCHECK(time_ntp_us >= 0);  // Time before year 1900 is unsupported.

  // Convert seconds to uint32 through uint64 for a well-defined cast.
  // A wrap around, which will happen in 2036, is expected for NTP time.
  uint32_t ntp_seconds =
      static_cast<uint64_t>(time_ntp_us / kNumMicrosecsPerSec);

  // Scale fractions of the second to NTP resolution.
  constexpr int64_t kNtpFractionsInSecond = 1LL << 32;
  int64_t us_fractions = time_ntp_us % kNumMicrosecsPerSec;
  uint32_t ntp_fractions =
      us_fractions * kNtpFractionsInSecond / kNumMicrosecsPerSec;

  return {ntp_seconds, ntp_fractions};
}

}  // namespace

Timestamp Clock::NtpToUtc(NtpTime ntp_time) {
  if (!ntp_time.Valid()) {
    return Timestamp::MinusInfinity();
  }
  // Seconds since UTC epoch.
  int64_t time = ntp_time.seconds() - kNtpJan1970;
  // Microseconds since UTC epoch (not including NTP fraction)
  time = time * 1'000'000;
  // Fractions part of the NTP time, in microseconds.
  auto time_fraction = DivideRoundToNearest(
      int64_t{ntp_time.fractions()} * 1'000'000, NtpTime::kFractionsPerSecond);
  return Timestamp::Micros(time + time_fraction);
}

class RealTimeClock : public Clock {
 public:
  RealTimeClock() = default;

  Timestamp CurrentTime() override { return Timestamp::Micros(TimeMicros()); }

  NtpTime ConvertTimestampToNtpTime(Timestamp timestamp) override {
    return TimeMicrosToNtp(timestamp.us());
  }
};

Clock* Clock::GetRealTimeClock() {
  static Clock* const clock = new RealTimeClock();
  return clock;
}

SimulatedClock::SimulatedClock(int64_t initial_time_us)
    : time_us_(initial_time_us) {}

SimulatedClock::SimulatedClock(Timestamp initial_time)
    : SimulatedClock(initial_time.us()) {}

SimulatedClock::~SimulatedClock() = default;

Timestamp SimulatedClock::CurrentTime() {
  return Timestamp::Micros(time_us_.load(std::memory_order_relaxed));
}

NtpTime SimulatedClock::ConvertTimestampToNtpTime(Timestamp timestamp) {
  int64_t now_us = timestamp.us();
  uint32_t seconds = (now_us / 1'000'000) + kNtpJan1970;
  auto fractions =
      static_cast<uint32_t>(static_cast<double>(now_us % 1'000'000) *
                            kMagicNtpFractionalUnit / 1'000'000);
  return {seconds, fractions};
}

void SimulatedClock::AdvanceTimeMilliseconds(int64_t milliseconds) {
  AdvanceTime(TimeDelta::Millis(milliseconds));
}

void SimulatedClock::AdvanceTimeMicroseconds(int64_t microseconds) {
  AdvanceTime(TimeDelta::Micros(microseconds));
}

void SimulatedClock::AdvanceTime(TimeDelta delta) {
  time_us_.fetch_add(delta.us(), std::memory_order_relaxed);
}

}  // namespace base
}  // namespace ave
