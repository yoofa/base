/*
 * sequence_number_unwrapper_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/sequence_number_unwrapper.h"

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

// ============================================================
// SeqNumUnwrapper<uint16_t> (RtpSequenceNumberUnwrapper)
// ============================================================

TEST(SeqNumUnwrapperTest, FirstValuePreserved) {
  RtpSequenceNumberUnwrapper unwrapper;
  EXPECT_EQ(unwrapper.Unwrap(100), 100);
}

TEST(SeqNumUnwrapperTest, SimpleIncrement) {
  RtpSequenceNumberUnwrapper unwrapper;
  EXPECT_EQ(unwrapper.Unwrap(0), 0);
  EXPECT_EQ(unwrapper.Unwrap(1), 1);
  EXPECT_EQ(unwrapper.Unwrap(100), 100);
}

TEST(SeqNumUnwrapperTest, WrapAroundForward) {
  RtpSequenceNumberUnwrapper unwrapper;
  EXPECT_EQ(unwrapper.Unwrap(65534), 65534);
  EXPECT_EQ(unwrapper.Unwrap(65535), 65535);
  EXPECT_EQ(unwrapper.Unwrap(0), 65536);  // wraps forward
  EXPECT_EQ(unwrapper.Unwrap(1), 65537);
}

TEST(SeqNumUnwrapperTest, WrapAroundBackward) {
  RtpSequenceNumberUnwrapper unwrapper;
  EXPECT_EQ(unwrapper.Unwrap(10), 10);
  EXPECT_EQ(unwrapper.Unwrap(5), 5);  // goes backward
}

TEST(SeqNumUnwrapperTest, MultipleWraps) {
  RtpSequenceNumberUnwrapper unwrapper;
  int64_t last = unwrapper.Unwrap(0);
  for (int i = 1; i < 200000; ++i) {
    int64_t cur = unwrapper.Unwrap(static_cast<uint16_t>(i));
    EXPECT_EQ(cur, last + 1);
    last = cur;
  }
  EXPECT_EQ(last, 199999);
}

TEST(SeqNumUnwrapperTest, ResetToInitialState) {
  RtpSequenceNumberUnwrapper unwrapper;
  unwrapper.Unwrap(100);
  unwrapper.Unwrap(200);
  unwrapper.Reset();
  // After reset, first Unwrap returns the raw value
  EXPECT_EQ(unwrapper.Unwrap(50), 50);
}

TEST(SeqNumUnwrapperTest, PeekDoesNotChangeState) {
  RtpSequenceNumberUnwrapper unwrapper;
  unwrapper.Unwrap(100);
  int64_t peeked = unwrapper.PeekUnwrap(101);
  EXPECT_EQ(peeked, 101);
  // Subsequent Unwrap should still see 101 as next
  EXPECT_EQ(unwrapper.Unwrap(101), 101);
}

TEST(SeqNumUnwrapperTest, PeekBeforeFirstUnwrap) {
  RtpSequenceNumberUnwrapper unwrapper;
  EXPECT_EQ(unwrapper.PeekUnwrap(42), 42);
  // State not changed, so Unwrap still starts fresh
  EXPECT_EQ(unwrapper.Unwrap(42), 42);
}

TEST(SeqNumUnwrapperTest, PeekWithWrap) {
  RtpSequenceNumberUnwrapper unwrapper;
  unwrapper.Unwrap(65535);
  int64_t peeked = unwrapper.PeekUnwrap(0);
  EXPECT_EQ(peeked, 65536);
  // Actual Unwrap should give same
  EXPECT_EQ(unwrapper.Unwrap(0), 65536);
}

// ============================================================
// SeqNumUnwrapper<uint32_t> (RtpTimestampUnwrapper)
// ============================================================

TEST(RtpTimestampUnwrapperTest, FirstValue) {
  RtpTimestampUnwrapper unwrapper;
  EXPECT_EQ(unwrapper.Unwrap(1000u), 1000);
}

TEST(RtpTimestampUnwrapperTest, WrapAround) {
  RtpTimestampUnwrapper unwrapper;
  uint32_t kMax = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(unwrapper.Unwrap(kMax), static_cast<int64_t>(kMax));
  EXPECT_EQ(unwrapper.Unwrap(0u), static_cast<int64_t>(kMax) + 1);
}

TEST(RtpTimestampUnwrapperTest, ResetAndRestart) {
  RtpTimestampUnwrapper unwrapper;
  unwrapper.Unwrap(5000u);
  unwrapper.Unwrap(6000u);
  unwrapper.Reset();
  EXPECT_EQ(unwrapper.Unwrap(0u), 0);
}

// ============================================================
// SeqNumUnwrapper with explicit modulus M
// ============================================================

TEST(SeqNumUnwrapperWithMTest, WrapsAtM) {
  SeqNumUnwrapper<uint8_t, 10> unwrapper;
  EXPECT_EQ(unwrapper.Unwrap(0), 0);
  EXPECT_EQ(unwrapper.Unwrap(5), 5);
  EXPECT_EQ(unwrapper.Unwrap(9), 9);
  EXPECT_EQ(unwrapper.Unwrap(0), 10);  // wraps forward at 10
  EXPECT_EQ(unwrapper.Unwrap(1), 11);
}

}  // namespace
}  // namespace base
}  // namespace ave
