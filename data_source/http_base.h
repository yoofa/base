/*
 * http_base.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef HTTP_BASE_H
#define HTTP_BASE_H

#include <list>
#include <mutex>
#include <unordered_map>

#include "base/constructor_magic.h"
#include "base/errors.h"

namespace ave {

class HTTPBase {
 public:
  enum Flags {
    // Don't log any URLs.
    kFlagIncognito = 1
  };
  HTTPBase();
  virtual ~HTTPBase() = default;

  virtual status_t Connect(
      const char* uri,
      const std::unordered_map<std::string, std::string>& headers,
      off64_t offset) = 0;

  virtual void Disconnect() = 0;
  // Returns true if bandwidth could successfully be estimated,
  // false otherwise.
  virtual bool EstimateBandwidth(uint32_t* bandwidth_bps);

  virtual status_t GetEstimatedBandwidthKbps(uint32_t* kbps);

  virtual status_t SetBandwidthStatCollectFreq(uint32_t freq_ms);

  virtual void SetBandwidthHistorySize(size_t num_history_items);

  virtual std::string toString() { return name_; }

 protected:
  virtual void AddBandwidthMeasurement(size_t num_bytes, int64_t delay_us);
  std::string name_;

 private:
  struct BandwidthEntry {
    int64_t delay_us;
    size_t num_bytes;
  };

  std::mutex lock_;

  std::list<BandwidthEntry> band_width_history_;
  size_t num_bandwidth_history_items_;
  int64_t total_transfer_time_us_;
  size_t total_transfer_bytes_;
  size_t max_bandwidth_history_items_;

  enum {
    kMinBandwidthCollectFreqMs = 1000,   // 1 second
    kMaxBandwidthCollectFreqMs = 60000,  // one minute
  };

  // int64_t mPrevBandwidthMeasureTimeUs;
  // int32_t mPrevEstimatedBandWidthKbps;
  // int32_t mBandWidthCollectFreqMs;
  uint64_t prev_bandwidth_measurement_time_us_;
  uint32_t prev_estimated_bandwidth_kbps_;
  uint32_t bandwidth_collect_freq_ms_;

  AVE_DISALLOW_COPY_AND_ASSIGN(HTTPBase);
};

}  // namespace ave

#endif /* !HTTP_BASE_H */
