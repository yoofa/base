/*
 * mod_ops_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/mod_ops.h"

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

// ============================================================
// Add<M> tests
// ============================================================

TEST(ModOpsAddTest, BasicAddition) {
  EXPECT_EQ((Add<10>(0, 5)), 5u);
  EXPECT_EQ((Add<10>(3, 4)), 7u);
}

TEST(ModOpsAddTest, Wraparound) {
  EXPECT_EQ((Add<10>(7, 5)), 2u);  // (7+5) % 10 = 2
  EXPECT_EQ((Add<10>(9, 1)), 0u);
  EXPECT_EQ((Add<256>(200, 100)), 44u);
}

TEST(ModOpsAddTest, AddZero) {
  EXPECT_EQ((Add<10>(5, 0)), 5u);
  EXPECT_EQ((Add<10>(0, 0)), 0u);
}

TEST(ModOpsAddTest, AddModuloItself) {
  EXPECT_EQ((Add<10>(5, 10)), 5u);  // b % M == 0
}

// ============================================================
// Subtract<M> tests
// ============================================================

TEST(ModOpsSubtractTest, BasicSubtraction) {
  EXPECT_EQ((Subtract<10>(5, 3)), 2u);
  EXPECT_EQ((Subtract<10>(9, 0)), 9u);
}

TEST(ModOpsSubtractTest, Wraparound) {
  EXPECT_EQ((Subtract<10>(2, 5)), 7u);  // (10 - (5-2)) = 7
  EXPECT_EQ((Subtract<10>(0, 1)), 9u);
}

TEST(ModOpsSubtractTest, SubtractZero) {
  EXPECT_EQ((Subtract<10>(5, 0)), 5u);
}

// ============================================================
// ForwardDiff tests
// ============================================================

TEST(ForwardDiffTest, SameValue) {
  EXPECT_EQ(ForwardDiff(uint8_t{5}, uint8_t{5}), uint8_t{0});
}

TEST(ForwardDiffTest, ForwardIncrease) {
  EXPECT_EQ(ForwardDiff(uint8_t{0}, uint8_t{10}), uint8_t{10});
  EXPECT_EQ(ForwardDiff(uint8_t{100}, uint8_t{200}), uint8_t{100});
}

TEST(ForwardDiffTest, WrapAround) {
  // x=253, y=2: forward diff = 5
  EXPECT_EQ(ForwardDiff(uint8_t{253}, uint8_t{2}), uint8_t{5});
  // y=2, x=253: forward diff = 251
  EXPECT_EQ(ForwardDiff(uint8_t{2}, uint8_t{253}), uint8_t{251});
}

TEST(ForwardDiffTest, WithModulusM) {
  // ForwardDiff<uint8_t, 10>(7, 2) = 2-7 wraps: 10-(7-2)=5
  EXPECT_EQ((ForwardDiff<uint8_t, 10>(7, 2)), uint8_t{5});
  EXPECT_EQ((ForwardDiff<uint8_t, 10>(2, 7)), uint8_t{5});
}

TEST(ForwardDiffTest, Uint16Wraparound) {
  constexpr uint16_t kMax = std::numeric_limits<uint16_t>::max();
  EXPECT_EQ(ForwardDiff(kMax, uint16_t{0}), uint16_t{1});
}

// ============================================================
// ReverseDiff tests
// ============================================================

TEST(ReverseDiffTest, SameValue) {
  EXPECT_EQ(ReverseDiff(uint8_t{5}, uint8_t{5}), uint8_t{0});
}

TEST(ReverseDiffTest, ReverseDecrease) {
  EXPECT_EQ(ReverseDiff(uint8_t{10}, uint8_t{0}), uint8_t{10});
  EXPECT_EQ(ReverseDiff(uint8_t{200}, uint8_t{100}), uint8_t{100});
}

TEST(ReverseDiffTest, WrapAround) {
  // ReverseDiff(y=2, x=253): reverse diff going backward from 2 to 253 = 5
  EXPECT_EQ(ReverseDiff(uint8_t{2}, uint8_t{253}), uint8_t{5});
  // ReverseDiff(x=253, y=2): reverse diff going backward from 253 to 2 = 251
  EXPECT_EQ(ReverseDiff(uint8_t{253}, uint8_t{2}), uint8_t{251});
}

// ============================================================
// MinDiff tests
// ============================================================

TEST(MinDiffTest, SameValue) {
  EXPECT_EQ(MinDiff(uint8_t{5}, uint8_t{5}), uint8_t{0});
}

TEST(MinDiffTest, ShortPath) {
  // ForwardDiff(10,15)=5, ReverseDiff(10,15)=251 => min=5
  EXPECT_EQ(MinDiff(uint8_t{10}, uint8_t{15}), uint8_t{5});
}

TEST(MinDiffTest, SymmetricResult) {
  // min distance should be the same regardless of direction
  uint8_t a = 10, b = 240;
  EXPECT_EQ(MinDiff(a, b), MinDiff(b, a));
}

TEST(MinDiffTest, HalfwayPoint) {
  // 0 and 128: forward=128, reverse=128 => min=128
  EXPECT_EQ(MinDiff(uint8_t{0}, uint8_t{128}), uint8_t{128});
}

TEST(MinDiffTest, WithExplicitModulus) {
  // ForwardDiff<uint8_t,10>(3,8)=5, ReverseDiff<uint8_t,10>(3,8)=5 => min=5
  EXPECT_EQ((MinDiff<uint8_t, 10>(3, 8)), uint8_t{5});
}

}  // namespace
}  // namespace base
}  // namespace ave
