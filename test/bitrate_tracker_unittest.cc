/*
 * bitrate_tracker_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/bitrate_tracker.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

TEST(BitrateTrackerTest, BasicBitrate) {
  BitrateTracker tracker(TimeDelta::Millis(1000));

  // At start, rate is unavailable.
  EXPECT_FALSE(tracker.Rate(Timestamp::Millis(0)).has_value());

  tracker.Update(1000, Timestamp::Millis(0));
  tracker.Update(1000, Timestamp::Millis(100));  // 100ms passed

  // Active window 101ms, bytes 2000 => bits 16000.
  // Rate = 16000 / 101 * 1000 = 158415 bps.
  auto rate = tracker.Rate(Timestamp::Millis(100));
  EXPECT_TRUE(rate.has_value());
  EXPECT_GT(rate->bps(), 150000);
  EXPECT_LT(rate->bps(), 160000);
}

TEST(BitrateTrackerTest, SetWindowSize) {
  BitrateTracker tracker(TimeDelta::Millis(1000));
  EXPECT_TRUE(
      tracker.SetWindowSize(TimeDelta::Millis(500), Timestamp::Millis(0)));
  EXPECT_FALSE(
      tracker.SetWindowSize(TimeDelta::Millis(1500), Timestamp::Millis(0)));
}

TEST(BitrateTrackerTest, Reset) {
  BitrateTracker tracker(TimeDelta::Millis(1000));
  tracker.Update(1000, Timestamp::Millis(0));
  tracker.Update(1000, Timestamp::Millis(100));
  EXPECT_TRUE(tracker.Rate(Timestamp::Millis(100)).has_value());

  tracker.Reset();
  EXPECT_FALSE(tracker.Rate(Timestamp::Millis(100)).has_value());
}

}  // namespace
}  // namespace base
}  // namespace ave
