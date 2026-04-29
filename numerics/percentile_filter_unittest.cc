/*
 * percentile_filter_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/percentile_filter.h"
#include "base/numerics/moving_percentile_filter.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

// ============================================================
// PercentileFilter tests
// ============================================================

TEST(PercentileFilterTest, EmptyReturnsDefault) {
  PercentileFilter<int> f(0.5f);
  EXPECT_EQ(f.GetPercentileValue(), 0);
}

TEST(PercentileFilterTest, SingleElement) {
  PercentileFilter<int> f(0.5f);
  f.Insert(42);
  EXPECT_EQ(f.GetPercentileValue(), 42);
}

TEST(PercentileFilterTest, MedianOddCount) {
  PercentileFilter<int> f(0.5f);
  f.Insert(3);
  f.Insert(1);
  f.Insert(5);
  f.Insert(2);
  f.Insert(4);
  // sorted: [1,2,3,4,5], 50th percentile index = floor(0.5*4) = 2 => value=3
  EXPECT_EQ(f.GetPercentileValue(), 3);
}

TEST(PercentileFilterTest, MedianEvenCount) {
  PercentileFilter<int> f(0.5f);
  f.Insert(1);
  f.Insert(2);
  f.Insert(3);
  f.Insert(4);
  // sorted: [1,2,3,4], index = floor(0.5*3) = 1 => value=2
  EXPECT_EQ(f.GetPercentileValue(), 2);
}

TEST(PercentileFilterTest, MinPercentile) {
  PercentileFilter<int> f(0.0f);
  f.Insert(10);
  f.Insert(5);
  f.Insert(20);
  EXPECT_EQ(f.GetPercentileValue(), 5);
}

TEST(PercentileFilterTest, MaxPercentile) {
  PercentileFilter<int> f(1.0f);
  f.Insert(10);
  f.Insert(5);
  f.Insert(20);
  EXPECT_EQ(f.GetPercentileValue(), 20);
}

TEST(PercentileFilterTest, EraseExistingElement) {
  PercentileFilter<int> f(0.5f);
  f.Insert(1);
  f.Insert(2);
  f.Insert(3);
  EXPECT_TRUE(f.Erase(2));
  // After erasing 2: [1,3], median index = floor(0.5*1) = 0 => value=1
  EXPECT_EQ(f.GetPercentileValue(), 1);
}

TEST(PercentileFilterTest, EraseNonExistentElement) {
  PercentileFilter<int> f(0.5f);
  f.Insert(1);
  EXPECT_FALSE(f.Erase(99));
  EXPECT_EQ(f.GetPercentileValue(), 1);  // unchanged
}

TEST(PercentileFilterTest, ResetClearsAll) {
  PercentileFilter<int> f(0.5f);
  f.Insert(10);
  f.Insert(20);
  f.Reset();
  EXPECT_EQ(f.GetPercentileValue(), 0);  // empty -> default
}

TEST(PercentileFilterTest, DuplicateValues) {
  PercentileFilter<int> f(0.5f);
  f.Insert(5);
  f.Insert(5);
  f.Insert(5);
  EXPECT_EQ(f.GetPercentileValue(), 5);
}

TEST(PercentileFilterTest, FloatValues) {
  PercentileFilter<float> f(0.5f);
  f.Insert(1.0f);
  f.Insert(2.0f);
  f.Insert(3.0f);
  EXPECT_FLOAT_EQ(f.GetPercentileValue(), 2.0f);
}

// ============================================================
// MovingPercentileFilter tests
// ============================================================

TEST(MovingPercentileFilterTest, SingleElement) {
  MovingPercentileFilter<int> f(0.5f, 5);
  f.Insert(42);
  EXPECT_EQ(f.GetFilteredValue(), 42);
  EXPECT_EQ(f.GetNumberOfSamplesStored(), 1u);
}

TEST(MovingPercentileFilterTest, WindowEviction) {
  MovingPercentileFilter<int> f(0.5f, 3);
  f.Insert(10);
  f.Insert(20);
  f.Insert(30);
  EXPECT_EQ(f.GetNumberOfSamplesStored(), 3u);
  // Insert 4th: evicts 10, window = [20,30,40]
  f.Insert(40);
  EXPECT_EQ(f.GetNumberOfSamplesStored(), 3u);
}

TEST(MovingPercentileFilterTest, MedianOverWindow) {
  // Window size 3, median
  MovingPercentileFilter<int> f(0.5f, 3);
  f.Insert(1);
  f.Insert(5);
  f.Insert(3);
  // window [1,3,5] sorted, index=floor(0.5*2)=1 => 3
  EXPECT_EQ(f.GetFilteredValue(), 3);
}

TEST(MovingPercentileFilterTest, SlidingWindow) {
  MovingPercentileFilter<int> f(0.5f, 3);
  f.Insert(1);
  f.Insert(2);
  f.Insert(3);
  f.Insert(100);  // evicts 1, window=[2,3,100], median index=1 => 3
  EXPECT_EQ(f.GetFilteredValue(), 3);
}

TEST(MovingPercentileFilterTest, ResetAndReuse) {
  MovingPercentileFilter<int> f(0.5f, 3);
  f.Insert(10);
  f.Insert(20);
  f.Reset();
  EXPECT_EQ(f.GetNumberOfSamplesStored(), 0u);
  f.Insert(5);
  EXPECT_EQ(f.GetFilteredValue(), 5);
}

// ============================================================
// MovingMedianFilter (convenience subclass)
// ============================================================

TEST(MovingMedianFilterTest, BasicMedian) {
  MovingMedianFilter<int> f(5);
  f.Insert(3);
  f.Insert(1);
  f.Insert(5);
  f.Insert(2);
  f.Insert(4);
  // window [1,2,3,4,5] (sorted), index=floor(0.5*4)=2 => 3
  EXPECT_EQ(f.GetFilteredValue(), 3);
}

TEST(MovingMedianFilterTest, SlidingRemovesOldest) {
  MovingMedianFilter<int> f(3);
  f.Insert(10);
  f.Insert(20);
  f.Insert(30);
  f.Insert(1);  // evicts 10, window=[1,20,30], sorted=[1,20,30], index=1=>20
  EXPECT_EQ(f.GetFilteredValue(), 20);
}

}  // namespace
}  // namespace base
}  // namespace ave
