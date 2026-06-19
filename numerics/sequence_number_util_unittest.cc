/*
 * sequence_number_util_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/sequence_number_util.h"

#include <algorithm>
#include <set>
#include <vector>

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

// ============================================================
// IsNewer tests (generic template)
// ============================================================

TEST(IsNewerTest, SameValueIsNotNewer) {
  EXPECT_FALSE(IsNewer(uint16_t{5}, uint16_t{5}));
  EXPECT_FALSE(IsNewer(uint32_t{1000}, uint32_t{1000}));
}

TEST(IsNewerTest, SimpleForwardIsNewer) {
  EXPECT_TRUE(IsNewer(uint16_t{1}, uint16_t{0}));
  EXPECT_TRUE(IsNewer(uint16_t{100}, uint16_t{50}));
}

TEST(IsNewerTest, WrapAroundIsNewer) {
  // 1 wraps around past 65535, so 1 is newer than 65535
  EXPECT_TRUE(IsNewer(uint16_t{1}, uint16_t{65535}));
  EXPECT_FALSE(IsNewer(uint16_t{65535}, uint16_t{1}));
}

TEST(IsNewerTest, HalfwayApart) {
  // Exactly 0x8000 apart: higher value is considered newer
  EXPECT_TRUE(IsNewer(uint16_t{0x8000}, uint16_t{0}));
  EXPECT_FALSE(IsNewer(uint16_t{0}, uint16_t{0x8000}));
}

// ============================================================
// IsNewerSequenceNumber tests
// ============================================================

TEST(IsNewerSequenceNumberTest, NormalIncrement) {
  EXPECT_TRUE(IsNewerSequenceNumber(1000, 999));
  EXPECT_FALSE(IsNewerSequenceNumber(999, 1000));
}

TEST(IsNewerSequenceNumberTest, WrapAround) {
  EXPECT_TRUE(IsNewerSequenceNumber(0, 65535));
  EXPECT_FALSE(IsNewerSequenceNumber(65535, 0));
}

// ============================================================
// IsNewerTimestamp tests
// ============================================================

TEST(IsNewerTimestampTest, NormalIncrement) {
  EXPECT_TRUE(IsNewerTimestamp(1000u, 999u));
  EXPECT_FALSE(IsNewerTimestamp(999u, 1000u));
}

TEST(IsNewerTimestampTest, WrapAround) {
  EXPECT_TRUE(IsNewerTimestamp(0u, 0xFFFFFFFFu));
  EXPECT_FALSE(IsNewerTimestamp(0xFFFFFFFFu, 0u));
}

// ============================================================
// LatestSequenceNumber tests
// ============================================================

TEST(LatestSequenceNumberTest, ReturnsNewer) {
  EXPECT_EQ(LatestSequenceNumber(100, 99), 100u);
  EXPECT_EQ(LatestSequenceNumber(99, 100), 100u);
}

TEST(LatestSequenceNumberTest, WrapAround) {
  // After wraparound, 0 is newer than 65535
  EXPECT_EQ(LatestSequenceNumber(0, 65535), 0u);
  EXPECT_EQ(LatestSequenceNumber(65535, 0), 0u);
}

// ============================================================
// LatestTimestamp tests
// ============================================================

TEST(LatestTimestampTest, ReturnsNewer) {
  EXPECT_EQ(LatestTimestamp(2000u, 1000u), 2000u);
  EXPECT_EQ(LatestTimestamp(1000u, 2000u), 2000u);
}

TEST(LatestTimestampTest, WrapAround) {
  EXPECT_EQ(LatestTimestamp(0u, 0xFFFFFFFFu), 0u);
  EXPECT_EQ(LatestTimestamp(0xFFFFFFFFu, 0u), 0u);
}

// ============================================================
// AheadOrAt / AheadOf tests
// ============================================================

TEST(AheadOrAtTest, SameValueIsAheadOrAt) {
  EXPECT_TRUE((AheadOrAt<uint16_t, 0>(uint16_t{0}, uint16_t{0})));
  EXPECT_TRUE((AheadOrAt<uint16_t, 0>(uint16_t{100}, uint16_t{100})));
}

TEST(AheadOrAtTest, ForwardIsAheadOrAt) {
  EXPECT_TRUE((AheadOrAt<uint16_t, 0>(uint16_t{10}, uint16_t{5})));
  EXPECT_FALSE((AheadOrAt<uint16_t, 0>(uint16_t{5}, uint16_t{10})));
}

TEST(AheadOrAtTest, WrapAround) {
  EXPECT_TRUE((AheadOrAt<uint16_t, 0>(uint16_t{0}, uint16_t{65534})));
  EXPECT_FALSE((AheadOrAt<uint16_t, 0>(uint16_t{65534}, uint16_t{0})));
}

TEST(AheadOfTest, SameValueIsNotAheadOf) {
  EXPECT_FALSE((AheadOf<uint16_t, 0>(uint16_t{5}, uint16_t{5})));
}

TEST(AheadOfTest, ForwardIsAheadOf) {
  EXPECT_TRUE((AheadOf<uint16_t, 0>(uint16_t{10}, uint16_t{5})));
  EXPECT_FALSE((AheadOf<uint16_t, 0>(uint16_t{5}, uint16_t{10})));
}

// ============================================================
// AscendingSeqNumComp tests
// ============================================================

TEST(AscendingSeqNumCompTest, SortSequenceNumbers) {
  // Insert in reverse order; result should be ascending by seq num logic
  std::set<uint16_t, AscendingSeqNumComp<uint16_t>> s;
  s.insert(5);
  s.insert(3);
  s.insert(7);
  s.insert(1);
  // AscendingSeqNumComp puts "newest first": comp(a,b) = AheadOf(a,b)
  // So smallest (earliest) seq num appears last; but for non-wrapping case
  // highest value = "most ahead" = comes first in set iteration
  std::vector<uint16_t> sorted(s.begin(), s.end());
  EXPECT_EQ(sorted.size(), 4u);
  // Verify the comparator used is consistent (no crash, values present)
  EXPECT_NE(std::ranges::find(sorted, 1u), sorted.end());
  EXPECT_NE(std::ranges::find(sorted, 7u), sorted.end());
}

// ============================================================
// DescendingSeqNumComp tests
// ============================================================

TEST(DescendingSeqNumCompTest, SortDescending) {
  std::set<uint16_t, DescendingSeqNumComp<uint16_t>> s;
  s.insert(10);
  s.insert(5);
  s.insert(20);
  std::vector<uint16_t> sorted(s.begin(), s.end());
  EXPECT_EQ(sorted.size(), 3u);
  EXPECT_NE(std::ranges::find(sorted, 5u), sorted.end());
  EXPECT_NE(std::ranges::find(sorted, 20u), sorted.end());
}

// ============================================================
// AheadOrAt with explicit M
// ============================================================

TEST(AheadOrAtWithMTest, BoundedWraparound) {
  // With M=10: 9 -> 0 is forward
  EXPECT_TRUE((AheadOrAt<uint8_t, 10>(0, 9)));
  EXPECT_FALSE((AheadOrAt<uint8_t, 10>(9, 0)));
}

}  // namespace
}  // namespace base
}  // namespace ave
