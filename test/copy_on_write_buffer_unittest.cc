/*
 * copy_on_write_buffer_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/copy_on_write_buffer.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

TEST(CopyOnWriteBufferTest, DefaultConstructor) {
  CopyOnWriteBuffer buf;
  EXPECT_TRUE(buf.empty());
  EXPECT_EQ(buf.size(), 0u);
  EXPECT_EQ(buf.capacity(), 0u);
  EXPECT_EQ(buf.data(), nullptr);
}

TEST(CopyOnWriteBufferTest, SizeConstructor) {
  CopyOnWriteBuffer buf(10);
  EXPECT_FALSE(buf.empty());
  EXPECT_EQ(buf.size(), 10u);
  EXPECT_GE(buf.capacity(), 10u);
}

TEST(CopyOnWriteBufferTest, CopyConstructorSharesData) {
  CopyOnWriteBuffer buf1(10);
  buf1.MutableData()[0] = 42;

  CopyOnWriteBuffer buf2(buf1);
  EXPECT_EQ(buf1.data(), buf2.data());  // Shared data
  EXPECT_EQ(buf2[0], 42);
}

TEST(CopyOnWriteBufferTest, MoveConstructor) {
  CopyOnWriteBuffer buf1(10);
  buf1.MutableData()[0] = 42;
  const uint8_t* ptr = buf1.data();

  CopyOnWriteBuffer buf2(std::move(buf1));
  EXPECT_EQ(buf2.data(), ptr);
  EXPECT_EQ(buf2.size(), 10u);
  EXPECT_EQ(buf1.size(), 0u);
}

TEST(CopyOnWriteBufferTest, StringConstructor) {
  std::string s = "hello";
  CopyOnWriteBuffer buf(s);
  EXPECT_EQ(buf.size(), 5u);
  EXPECT_EQ(buf.data()[0], 'h');
}

TEST(CopyOnWriteBufferTest, UnsharesOnMutableData) {
  CopyOnWriteBuffer buf1(10);
  buf1.MutableData()[0] = 1;

  CopyOnWriteBuffer buf2(buf1);
  EXPECT_EQ(buf1.data(), buf2.data());

  buf2.MutableData()[0] = 2;  // Should unshare
  EXPECT_NE(buf1.data(), buf2.data());
  EXPECT_EQ(buf1[0], 1);
  EXPECT_EQ(buf2[0], 2);
}

TEST(CopyOnWriteBufferTest, SetData) {
  CopyOnWriteBuffer buf;
  // NOLINTNEXTLINE
  const uint8_t data[] = {1, 2, 3};
  buf.SetData(data, 3);
  EXPECT_EQ(buf.size(), 3u);
  EXPECT_EQ(buf[0], 1);
}

TEST(CopyOnWriteBufferTest, AppendData) {
  CopyOnWriteBuffer buf;
  // NOLINTNEXTLINE
  const uint8_t data1[] = {1, 2};
  // NOLINTNEXTLINE
  const uint8_t data2[] = {3, 4};

  buf.AppendData(data1, 2);
  buf.AppendData(data2, 2);

  EXPECT_EQ(buf.size(), 4u);
  EXPECT_EQ(buf[0], 1);
  EXPECT_EQ(buf[2], 3);
  EXPECT_EQ(buf[3], 4);
}

}  // namespace
}  // namespace base
}  // namespace ave
