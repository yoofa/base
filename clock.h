/*
 * clock.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_BASE_CLOCK_H_
#define AVE_BASE_CLOCK_H_

#include <atomic>
#include <cstdint>

#include "base/units/time_delta.h"
#include "base/units/timestamp.h"

namespace ave {
namespace base {

// NtpTime represents an NTP timestamp (RFC 1305)
class NtpTime {
 public:
  static constexpr uint64_t kFractionsPerSecond = 0x100000000;

  NtpTime() : value_(0) {}
  explicit NtpTime(uint64_t value) : value_(value) {}
  NtpTime(uint32_t seconds, uint32_t fractions)
      : value_(seconds * kFractionsPerSecond + fractions) {}

  NtpTime(const NtpTime&) = default;
  NtpTime& operator=(const NtpTime&) = default;
  explicit operator uint64_t() const { return value_; }

  void Set(uint32_t seconds, uint32_t fractions) {
    value_ = seconds * kFractionsPerSecond + fractions;
  }
  void Reset() { value_ = 0; }

  int64_t ToMs() const {
    static constexpr double kNtpFracPerMs = 4.294967296E6;  // 2^32 / 1000.
    const double frac_ms = static_cast<double>(fractions()) / kNtpFracPerMs;
    return 1000 * static_cast<int64_t>(seconds()) +
           static_cast<int64_t>(std::round(frac_ms + 0.5));
  }

  // NTP standard (RFC1305, section 3.1) explicitly state value 0 is invalid.
  bool Valid() const { return value_ != 0; }

  uint32_t seconds() const {
    return static_cast<uint32_t>(value_ / kFractionsPerSecond);
  }
  uint32_t fractions() const {
    return static_cast<uint32_t>(value_ % kFractionsPerSecond);
  }

 private:
  uint64_t value_;
};

inline bool operator==(const NtpTime& n1, const NtpTime& n2) {
  return static_cast<uint64_t>(n1) == static_cast<uint64_t>(n2);
}
inline bool operator!=(const NtpTime& n1, const NtpTime& n2) {
  return !(n1 == n2);
}

// January 1970, in NTP seconds.
constexpr uint32_t kNtpJan1970 = 2208988800UL;

// Magic NTP fractional unit.
constexpr double kMagicNtpFractionalUnit = 4.294967296E+9;

// A clock interface that allows reading of absolute and relative timestamps.
class Clock {
 public:
  virtual ~Clock() = default;

  // Return a timestamp relative to an unspecified epoch.
  virtual Timestamp CurrentTime() = 0;
  int64_t TimeInMilliseconds() { return CurrentTime().ms(); }
  int64_t TimeInMicroseconds() { return CurrentTime().us(); }

  // Retrieve an NTP absolute timestamp (with an epoch of Jan 1, 1900).
  NtpTime CurrentNtpTime() { return ConvertTimestampToNtpTime(CurrentTime()); }
  int64_t CurrentNtpInMilliseconds() { return CurrentNtpTime().ToMs(); }

  // Converts between a relative timestamp returned by this clock, to NTP time.
  virtual NtpTime ConvertTimestampToNtpTime(Timestamp timestamp) = 0;
  int64_t ConvertTimestampToNtpTimeInMilliseconds(int64_t timestamp_ms) {
    return ConvertTimestampToNtpTime(Timestamp::Millis(timestamp_ms)).ToMs();
  }

  // Converts NtpTime to a Timestamp with UTC epoch.
  // A `Minus Infinity` Timestamp is returned if the NtpTime is invalid.
  static Timestamp NtpToUtc(NtpTime ntp_time);

  // Returns an instance of the real-time system clock implementation.
  static Clock* GetRealTimeClock();
};

class SimulatedClock : public Clock {
 public:
  // The constructors assume an epoch of Jan 1, 1970.
  explicit SimulatedClock(int64_t initial_time_us);
  explicit SimulatedClock(Timestamp initial_time);
  ~SimulatedClock() override;

  // Return a timestamp with an epoch of Jan 1, 1970.
  Timestamp CurrentTime() override;

  NtpTime ConvertTimestampToNtpTime(Timestamp timestamp) override;

  // Advance the simulated clock with a given number of milliseconds or
  // microseconds.
  void AdvanceTimeMilliseconds(int64_t milliseconds);
  void AdvanceTimeMicroseconds(int64_t microseconds);
  void AdvanceTime(TimeDelta delta);

 private:
  std::atomic<int64_t> time_us_;
};

}  // namespace base
}  // namespace ave

#endif  // AVE_BASE_CLOCK_H_
