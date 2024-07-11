/*
 * http_base.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "http_base.h"

#include "base/logging.h"
#include "base/time_utils.h"

namespace ave {

HTTPBase::HTTPBase()
    : name_(std::string("HTTPBase<disconnected>")),
      num_bandwidth_history_items_(0),
      total_transfer_time_us_(0),
      total_transfer_bytes_(0),
      max_bandwidth_history_items_(0),
      prev_bandwidth_measurement_time_us_(0),
      prev_estimated_bandwidth_kbps_(0),
      bandwidth_collect_freq_ms_(0) {}

void HTTPBase::AddBandwidthMeasurement(size_t num_bytes, int64_t delay_us) {
  std::lock_guard<std::mutex> l(lock_);
  BandwidthEntry entry{};
  entry.delay_us = delay_us;
  entry.num_bytes = num_bytes;
  total_transfer_time_us_ += delay_us;
  total_transfer_bytes_ += num_bytes;

  band_width_history_.push_back(entry);
  if (++num_bandwidth_history_items_ <= max_bandwidth_history_items_) {
    return;
  }
  total_transfer_time_us_ -= band_width_history_.front().delay_us;
  total_transfer_bytes_ -= band_width_history_.front().num_bytes;
  band_width_history_.pop_front();
  --num_bandwidth_history_items_;

  int64_t now_us = base::TimeMicros();
  if (now_us - prev_bandwidth_measurement_time_us_ <
      bandwidth_collect_freq_ms_ * 1000LL) {
    return;
  }
  if (prev_bandwidth_measurement_time_us_ != 0) {
    prev_estimated_bandwidth_kbps_ =
        total_transfer_bytes_ * 8 / (total_transfer_time_us_ / 1000);
  }
  prev_bandwidth_measurement_time_us_ = now_us;
}

bool HTTPBase::EstimateBandwidth(uint32_t* bandwidth_bps) {
  std::lock_guard<std::mutex> l(lock_);
  // Do not do bandwidth estimation if we don't have enough samples, or
  // total bytes download are too small (<64K).
  // Bandwidth estimation from these samples can often shoot up and cause
  // unwanted bw adaption behaviors.
  if (num_bandwidth_history_items_ < 2 || total_transfer_bytes_ < 64 * 1024) {
    return false;
  }
  *bandwidth_bps =
      static_cast<uint32_t>((static_cast<double>(total_transfer_bytes_) * 8E6 /
                             static_cast<double>(total_transfer_time_us_)));
  return true;
}

status_t HTTPBase::GetEstimatedBandwidthKbps(uint32_t* kbps) {
  std::lock_guard<std::mutex> l(lock_);
  *kbps = prev_estimated_bandwidth_kbps_;
  return OK;
}

status_t HTTPBase::SetBandwidthStatCollectFreq(uint32_t freq_ms) {
  std::lock_guard<std::mutex> l(lock_);
  if (freq_ms < kMinBandwidthCollectFreqMs ||
      freq_ms > kMaxBandwidthCollectFreqMs) {
    AVE_LOG(LS_ERROR) << "Invalid bandwidth collection frequency: " << freq_ms
                      << "ms out of range [" << kMinBandwidthCollectFreqMs
                      << ", " << kMaxBandwidthCollectFreqMs << "]";
    return BAD_VALUE;
  }
  AVE_LOG(LS_INFO) << "Setting bandwidth collection frequency to " << freq_ms
                   << "ms";
  bandwidth_collect_freq_ms_ = freq_ms;
  return OK;
}

void HTTPBase::SetBandwidthHistorySize(size_t num_history_items) {
  std::lock_guard<std::mutex> l(lock_);
  max_bandwidth_history_items_ = num_history_items;
}

}  // namespace ave
