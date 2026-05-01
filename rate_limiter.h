/*
 * rate_limiter.h - Rate limiter for bitrate limiting
 *
 * Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 * Copyright (c) 2024 The aspect project authors. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source tree.
 */

#ifndef BASE_RATE_LIMITER_H_
#define BASE_RATE_LIMITER_H_

#include <cstddef>
#include <cstdint>
#include <mutex>

#include "base/clock.h"
#include "base/rate_statistics.h"

namespace ave {
namespace base {

// Class used to limit a bitrate, making sure the average does not exceed a
// maximum as measured over a sliding window. This class is thread safe; all
// methods will acquire (the same) lock before executing.
class RateLimiter {
 public:
  RateLimiter(Clock* clock, int64_t max_window_ms);

  RateLimiter() = delete;
  RateLimiter(const RateLimiter&) = delete;
  RateLimiter& operator=(const RateLimiter&) = delete;

  ~RateLimiter();

  // Try to use rate to send bytes. Returns true on success and if so updates
  // current rate.
  bool TryUseRate(size_t packet_size_bytes);

  // Set the maximum bitrate, in bps, that this limiter allows to send.
  void SetMaxRate(uint32_t max_rate_bps);

  // Set the window size over which to measure the current bitrate.
  // For example, in retransmissions, this is typically the RTT.
  // Returns true on success and false if window_size_ms is out of range.
  bool SetWindowSize(int64_t window_size_ms);

 private:
  Clock* const clock_;
  mutable std::mutex lock_;
  RateStatistics current_rate_;
  int64_t window_size_ms_;
  uint32_t max_rate_bps_;
};

}  // namespace base
}  // namespace ave

#endif  // BASE_RATE_LIMITER_H_
