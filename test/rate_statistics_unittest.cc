/*
 * rate_statistics_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/rate_statistics.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

TEST(RateStatisticsTest, InitialState) {
  RateStatistics stats(500, 8000.0f);
  EXPECT_FALSE(stats.Rate(0).has_value());
}

TEST(RateStatisticsTest, BasicRate) {
  RateStatistics stats(500, 8000.0f);
  stats.Update(1000, 0);
  // Single sample in window, rate unavailable.
  EXPECT_FALSE(stats.Rate(0).has_value());

  // Second sample makes rate available.
  stats.Update(1000, 1);
  // Active window size is 2 (0 to 1). Total count 2000.
  // Scale is 8000 / 2 = 4000. Rate = 2000 * 4000 = 8000000.
  EXPECT_EQ(stats.Rate(1).value_or(0), 8000000);
}

TEST(RateStatisticsTest, WindowSliding) {
  RateStatistics stats(10, 8000.0f);

  for (int i = 0; i < 10; ++i) {
    stats.Update(1000, i);
  }

  // At i=9, window is from 0 to 9, size = 10. Total count = 10 * 1000 = 10000.
  // Scale = 8000 / 10 = 800. Rate = 10000 * 800 = 8000000.
  EXPECT_EQ(stats.Rate(9).value_or(0), 8000000);

  // Move window by 5 ms. 5 samples drop out.
  // Window size is max 10.
  stats.Update(1000, 14);  // Now it's at 14. Oldest is 5.
  // Samples are at 5, 6, 7, 8, 9, 14. Total 6 samples.
  // Window size = 10 (since 14 - first_timestamp(0) >= 10, window is considered
  // full) Total count = 6 * 1000 = 6000. Scale = 8000 / 10 = 800. Rate =
  // 4800000.
  EXPECT_EQ(stats.Rate(14).value_or(0), 4800000);
}

TEST(RateStatisticsTest, SetWindowSize) {
  RateStatistics stats(500, 8000.0f);
  stats.Update(1000, 0);
  stats.Update(1000, 1);

  EXPECT_TRUE(stats.SetWindowSize(2, 2));
  EXPECT_FALSE(stats.SetWindowSize(600, 2));  // larger than max
  EXPECT_FALSE(stats.SetWindowSize(0, 2));
}

TEST(RateStatisticsTest, Reset) {
  RateStatistics stats(500, 8000.0f);
  stats.Update(1000, 0);
  stats.Update(1000, 1);
  EXPECT_TRUE(stats.Rate(1).has_value());

  stats.Reset();
  EXPECT_FALSE(stats.Rate(1).has_value());
}

}  // namespace
}  // namespace base
}  // namespace ave
