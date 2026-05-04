/*
 * buffer_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/buffer.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

TEST(BufferTest, DefaultConstructor) {
  Buffer buf;
  EXPECT_TRUE(buf.empty());
  EXPECT_EQ(buf.size(), 0u);
  EXPECT_EQ(buf.capacity(), 0u);
  EXPECT_EQ(buf.data(), nullptr);
}

TEST(BufferTest, SizeConstructor) {
  Buffer buf(10);
  EXPECT_FALSE(buf.empty());
  EXPECT_EQ(buf.size(), 10u);
  EXPECT_GE(buf.capacity(), 10u);
  EXPECT_NE(buf.data(), nullptr);
}

TEST(BufferTest, ArrayConstructor) {
  // NOLINTNEXTLINE
  const uint8_t data[] = {1, 2, 3, 4, 5};
  Buffer buf(data);
  EXPECT_EQ(buf.size(), 5u);
  EXPECT_EQ(buf[0], 1);
  EXPECT_EQ(buf[4], 5);
}

TEST(BufferTest, PointerAndSizeConstructor) {
  // NOLINTNEXTLINE
  const uint8_t data[] = {1, 2, 3, 4, 5};
  Buffer buf(data, 3);
  EXPECT_EQ(buf.size(), 3u);
  EXPECT_EQ(buf[0], 1);
  EXPECT_EQ(buf[2], 3);
}

TEST(BufferTest, SetData) {
  Buffer buf;
  // NOLINTNEXTLINE
  const uint8_t data[] = {1, 2, 3};
  buf.SetData(data);
  EXPECT_EQ(buf.size(), 3u);
  EXPECT_EQ(buf[0], 1);

  buf.SetData(data, 2);
  EXPECT_EQ(buf.size(), 2u);
  EXPECT_EQ(buf[1], 2);
}

TEST(BufferTest, AppendData) {
  Buffer buf;
  // NOLINTNEXTLINE
  const uint8_t data1[] = {1, 2};
  // NOLINTNEXTLINE
  const uint8_t data2[] = {3, 4};

  buf.AppendData(data1);
  buf.AppendData(data2);

  EXPECT_EQ(buf.size(), 4u);
  EXPECT_EQ(buf[0], 1);
  EXPECT_EQ(buf[2], 3);
  EXPECT_EQ(buf[3], 4);
}

TEST(BufferTest, SetSize) {
  Buffer buf(5);
  buf.SetSize(10);
  EXPECT_EQ(buf.size(), 10u);
  EXPECT_GE(buf.capacity(), 10u);
}

TEST(BufferTest, EnsureCapacity) {
  Buffer buf(5);
  buf.EnsureCapacity(100);
  EXPECT_EQ(buf.size(), 5u);
  EXPECT_GE(buf.capacity(), 100u);
}

TEST(BufferTest, Clear) {
  Buffer buf(5);
  buf.Clear();
  EXPECT_TRUE(buf.empty());
  EXPECT_EQ(buf.size(), 0u);
}

TEST(BufferTest, MoveConstructor) {
  Buffer buf1(5);
  buf1[0] = 42;

  Buffer buf2(std::move(buf1));
  EXPECT_EQ(buf2.size(), 5u);
  EXPECT_EQ(buf2[0], 42);
}

}  // namespace
}  // namespace base
}  // namespace ave
