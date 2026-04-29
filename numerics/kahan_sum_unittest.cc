/*
 * kahan_sum_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/numerics/kahan_sum.h"

#include <cmath>

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

// ============================================================
// KahanSum tests
// ============================================================

TEST(KahanSumTest, DefaultConstructorIsZero) {
  KahanSum<double> s;
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 0.0);
}

TEST(KahanSumTest, ConstructWithInitialValue) {
  KahanSum<double> s(5.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 5.0);
}

TEST(KahanSumTest, AddSingleValue) {
  KahanSum<double> s;
  s += 3.14;
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 3.14);
}

TEST(KahanSumTest, SumOfIntegers) {
  KahanSum<double> s;
  for (int i = 1; i <= 100; ++i) {
    s += static_cast<double>(i);
  }
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 5050.0);
}

TEST(KahanSumTest, CatastrophicCancellation) {
  // (BIG + SMALL - BIG) should not give 0 with compensated summation
  KahanSum<double> s;
  double big = 1e15;
  double small_val = 1.0;
  s += big;
  s += small_val;
  s += (-big);
  // With Kahan summation the small value should be preserved
  EXPECT_DOUBLE_EQ(static_cast<double>(s), small_val);
}

TEST(KahanSumTest, OperatorPlusReturnsNewSum) {
  KahanSum<double> s;
  KahanSum<double> s2 = s + 5.0;
  EXPECT_DOUBLE_EQ(static_cast<double>(s2), 5.0);
  // Original unchanged
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 0.0);
}

TEST(KahanSumTest, ResetToZero) {
  KahanSum<double> s;
  s += 42.0;
  s.Reset();
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 0.0);
}

TEST(KahanSumTest, FloatPrecision) {
  KahanSum<float> s;
  for (int i = 0; i < 1000; ++i) {
    s += 0.1f;
  }
  // Without Kahan, 1000 * 0.1f has large float error; with Kahan it's tighter
  EXPECT_NEAR(static_cast<float>(s), 100.0f, 0.01f);
}

// ============================================================
// NeumaierSum tests
// ============================================================

TEST(NeumaierSumTest, DefaultConstructorIsZero) {
  NeumaierSum<double> s;
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 0.0);
}

TEST(NeumaierSumTest, ConstructWithInitialValue) {
  NeumaierSum<double> s(7.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 7.0);
}

TEST(NeumaierSumTest, AddSingleValue) {
  NeumaierSum<double> s;
  s += 2.718;
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 2.718);
}

TEST(NeumaierSumTest, SumOfIntegers) {
  NeumaierSum<double> s;
  for (int i = 1; i <= 100; ++i) {
    s += static_cast<double>(i);
  }
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 5050.0);
}

TEST(NeumaierSumTest, InputLargerThanSum) {
  // Neumaier handles the case where input > running sum better than Kahan
  NeumaierSum<double> s;
  double small_init = 1.0;
  double big = 1e15;
  s += small_init;
  s += big;
  s += (-big);
  // Result should be close to small_init
  EXPECT_NEAR(static_cast<double>(s), small_init, 1e-6);
}

TEST(NeumaierSumTest, ResetToZero) {
  NeumaierSum<double> s;
  s += 99.0;
  s.Reset();
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 0.0);
}

TEST(NeumaierSumTest, OperatorPlusImmutable) {
  NeumaierSum<double> s;
  NeumaierSum<double> s2 = s + 10.0;
  EXPECT_DOUBLE_EQ(static_cast<double>(s2), 10.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(s), 0.0);
}

TEST(NeumaierSumTest, FloatPrecision) {
  NeumaierSum<float> s;
  for (int i = 0; i < 1000; ++i) {
    s += 0.1f;
  }
  EXPECT_NEAR(static_cast<float>(s), 100.0f, 0.01f);
}

}  // namespace
}  // namespace base
}  // namespace ave
