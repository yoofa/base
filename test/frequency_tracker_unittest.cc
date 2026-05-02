/*
 * frequency_tracker_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/frequency_tracker.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

TEST(FrequencyTrackerTest, BasicFrequency) {
  FrequencyTracker tracker(TimeDelta::Millis(1000));

  EXPECT_FALSE(tracker.Rate(Timestamp::Millis(0)).has_value());

  tracker.Update(Timestamp::Millis(0));
  tracker.Update(Timestamp::Millis(100));  // 2 frames in 100ms

  // Rate is 2 frames per 101 ms = 19.8 Hz
  auto rate = tracker.Rate(Timestamp::Millis(100));
  EXPECT_TRUE(rate.has_value());
  EXPECT_GT(rate->hertz(), 19);
  EXPECT_LT(rate->hertz(), 21);
}

TEST(FrequencyTrackerTest, Reset) {
  FrequencyTracker tracker(TimeDelta::Millis(1000));
  tracker.Update(Timestamp::Millis(0));
  tracker.Update(Timestamp::Millis(100));
  EXPECT_TRUE(tracker.Rate(Timestamp::Millis(100)).has_value());

  tracker.Reset();
  EXPECT_FALSE(tracker.Rate(Timestamp::Millis(100)).has_value());
}

}  // namespace
}  // namespace base
}  // namespace ave
