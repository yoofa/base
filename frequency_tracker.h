/*
 * frequency_tracker.h - Frequency tracker
 *
 * Copyright (c) 2024 The WebRTC project authors. All Rights Reserved.
 * Adapted for aspect video engine.
 */

#ifndef BASE_FREQUENCY_TRACKER_H_
#define BASE_FREQUENCY_TRACKER_H_

#include <cstddef>
#include <cstdint>
#include <optional>

#include "base/rate_statistics.h"
#include "base/units/frequency.h"
#include "base/units/timestamp.h"

namespace ave {
namespace base {

// Note: TimeDelta, Timestamp, and Frequency are already in the ave namespace,
// so no 'using' declarations are needed.

// Class to estimate frequency (e.g. frame rate) over running window.
// Timestamps used in Update() and Rate() must never decrease for two
// consecutive calls.
// This class is thread unsafe.
class FrequencyTracker {
 public:
  explicit FrequencyTracker(TimeDelta window_size)
      : impl_(window_size.ms(), /*scale=*/1000) {}

  FrequencyTracker(const FrequencyTracker&) = default;
  FrequencyTracker(FrequencyTracker&&) = default;
  FrequencyTracker& operator=(const FrequencyTracker&) = delete;
  FrequencyTracker& operator=(FrequencyTracker&&) = delete;

  ~FrequencyTracker() = default;

  // Reset instance to original state.
  void Reset() { impl_.Reset(); }

  // Update rate with a new data point, moving averaging window as needed.
  void Update(int64_t count, Timestamp now) { impl_.Update(count, now.ms()); }
  void Update(Timestamp now) { Update(1, now); }

  // Returns rate, moving averaging window as needed.
  // Returns nullopt when rate can't be measured.
  ::std::optional<Frequency> Rate(Timestamp now) const {
    auto rate = impl_.Rate(now.ms());
    if (rate.has_value()) {
      return Frequency::Hertz(*rate);
    }
    return ::std::nullopt;
  }

 private:
  mutable RateStatistics impl_;
};

}  // namespace base
}  // namespace ave

#endif  // BASE_FREQUENCY_TRACKER_H_
