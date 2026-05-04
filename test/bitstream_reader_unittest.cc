/*
 * bitstream_reader_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/buffer/bitstream_reader.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace {

TEST(BitstreamReaderTest, BasicInitialization) {
  // NOLINTNEXTLINE
  const uint8_t bytes[] = {0xA5};
  BitstreamReader reader(bytes);
  EXPECT_EQ(reader.RemainingBitCount(), 8);
  EXPECT_TRUE(reader.Ok());
}

TEST(BitstreamReaderTest, ReadBit) {
  // NOLINTNEXTLINE
  const uint8_t bytes[] = {0xA5};  // 10100101
  BitstreamReader reader(bytes);

  EXPECT_EQ(reader.ReadBit(), 1);
  EXPECT_EQ(reader.ReadBit(), 0);
  EXPECT_EQ(reader.ReadBit(), 1);
  EXPECT_EQ(reader.ReadBit(), 0);
  EXPECT_EQ(reader.ReadBit(), 0);
  EXPECT_EQ(reader.ReadBit(), 1);
  EXPECT_EQ(reader.ReadBit(), 0);
  EXPECT_EQ(reader.ReadBit(), 1);

  EXPECT_EQ(reader.RemainingBitCount(), 0);
  EXPECT_TRUE(reader.Ok());
}

TEST(BitstreamReaderTest, ReadBits) {
  // NOLINTNEXTLINE
  const uint8_t bytes[] = {0x12, 0x34};  // 00010010 00110100
  BitstreamReader reader(bytes);

  EXPECT_EQ(reader.ReadBits(4), 0x01u);
  EXPECT_EQ(reader.ReadBits(8), 0x23u);  // Next 8 bits: 0010 0011
  EXPECT_EQ(reader.ReadBits(4), 0x04u);  // Next 4 bits: 0100

  EXPECT_EQ(reader.RemainingBitCount(), 0);
  EXPECT_TRUE(reader.Ok());
}

TEST(BitstreamReaderTest, ConsumeBits) {
  // NOLINTNEXTLINE
  const uint8_t bytes[] = {0x12, 0x34};
  BitstreamReader reader(bytes);

  reader.ConsumeBits(4);
  EXPECT_EQ(reader.RemainingBitCount(), 12);
  EXPECT_EQ(reader.ReadBits(4), 0x02u);  // 0010
  EXPECT_TRUE(reader.Ok());
}

TEST(BitstreamReaderTest, ReadTyped) {
  // NOLINTNEXTLINE
  const uint8_t bytes[] = {0x12, 0x34, 0x56, 0x78};
  BitstreamReader reader(bytes);

  EXPECT_EQ(reader.Read<uint8_t>(), 0x12);
  EXPECT_EQ(reader.Read<uint16_t>(), 0x3456);
  EXPECT_TRUE(reader.Ok());
}

TEST(BitstreamReaderTest, OutOfBounds) {
  // NOLINTNEXTLINE
  const uint8_t bytes[] = {0x12};
  BitstreamReader reader(bytes);

  EXPECT_EQ(reader.ReadBits(10), 0u);  // Too many bits, returns 0 on error
  EXPECT_FALSE(reader.Ok());
}

}  // namespace
}  // namespace base
}  // namespace ave
