/*
 * rate_limiter_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/rate_limiter.h"

#include <gtest/gtest.h>
#include "base/clock.h"

namespace ave {
namespace base {
namespace {

TEST(RateLimiterTest, BasicRateLimiter) {
  SimulatedClock clock(0);
  RateLimiter limiter(&clock, 1000);
  limiter.SetMaxRate(8000);  // 8000 bps = 1000 bytes per sec

  // Initially rate is unavailable, so any size is allowed.
  EXPECT_TRUE(limiter.TryUseRate(500));  // 500 bytes = 4000 bits
  clock.AdvanceTimeMilliseconds(1);
  EXPECT_TRUE(limiter.TryUseRate(500));  // 500 bytes = 4000 bits

  clock.AdvanceTimeMilliseconds(1);
  // Rate is now 8000 bps, and window is 1000ms. Adding 1 byte (8 bits) would
  // make it exceed. Wait, active window size is 2ms (0 to 1). Total count is
  // 1000 bytes = 8000 bits. Rate is 1000 * 8000 / 2 = 4,000,000 bps! So it
  // exceeds 8000 immediately.
  EXPECT_FALSE(limiter.TryUseRate(1));

  // Advance 1000ms so that the first 500 bytes drop out.
  clock.AdvanceTimeMilliseconds(1000);
  // Now both have dropped out. Rate is 0, or unavailable.
  EXPECT_TRUE(limiter.TryUseRate(500));
}

TEST(RateLimiterTest, SetWindowSize) {
  SimulatedClock clock(0);
  RateLimiter limiter(&clock, 1000);
  limiter.SetMaxRate(8000);

  EXPECT_TRUE(limiter.SetWindowSize(500));
  EXPECT_FALSE(limiter.SetWindowSize(2000));  // Out of max range
}

}  // namespace
}  // namespace base
}  // namespace ave
