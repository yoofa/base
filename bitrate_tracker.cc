/*
 * bitrate_tracker.cc - Bitrate tracking over running window
 * Ported from WebRTC (rtc_base/bitrate_tracker.cc)
 *
 * Copyright (c) 2023 The WebRTC project authors. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the root of the source tree. An additional
 * intellectual property rights grant can be found in the file PATENTS.
 */

#include "base/bitrate_tracker.h"

#include <optional>

#include "base/rate_statistics.h"
#include "base/units/data_rate.h"

namespace ave {
namespace base {

using ::ave::base::Timestamp;

BitrateTracker::BitrateTracker(TimeDelta max_window_size)
    : impl_(max_window_size.ms(), RateStatistics::kBpsScale) {}

std::optional<DataRate> BitrateTracker::Rate(Timestamp now) const {
  if (std::optional<int64_t> rate = impl_.Rate(now.ms())) {
    return DataRate::BitsPerSec(*rate);
  }
  return std::nullopt;
}

bool BitrateTracker::SetWindowSize(TimeDelta window_size, Timestamp now) {
  return impl_.SetWindowSize(window_size.ms(), now.ms());
}

void BitrateTracker::Update(int64_t bytes, Timestamp now) {
  impl_.Update(bytes, now.ms());
}

}  // namespace base
}  // namespace ave
