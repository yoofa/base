/*
 * bit_buffer_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/buffer/bit_buffer.h"

#include <gtest/gtest.h>
#include <array>

namespace ave {
namespace base {
namespace {

TEST(BitBufferWriterTest, BasicInitialization) {
  std::array<uint8_t, 4> bytes = {0};
  BitBufferWriter writer(bytes.data(), 4);
  EXPECT_EQ(writer.RemainingBitCount(), 32u);

  size_t byte_offset = 0, bit_offset = 0;
  writer.GetCurrentOffset(&byte_offset, &bit_offset);
  EXPECT_EQ(byte_offset, 0u);
  EXPECT_EQ(bit_offset, 0u);
}

TEST(BitBufferWriterTest, WriteBits) {
  std::array<uint8_t, 2> bytes = {0};
  BitBufferWriter writer(bytes.data(), 2);

  EXPECT_TRUE(writer.WriteBits(0x0A, 4));  // Write 1010
  EXPECT_EQ(bytes[0], 0xA0);               // Top 4 bits

  EXPECT_TRUE(writer.WriteBits(0x05, 4));  // Write 0101
  EXPECT_EQ(bytes[0], 0xA5);               // Full byte

  EXPECT_EQ(writer.RemainingBitCount(), 8u);
}

TEST(BitBufferWriterTest, ConsumeBytesAndBits) {
  std::array<uint8_t, 4> bytes = {0};
  BitBufferWriter writer(bytes.data(), 4);

  EXPECT_TRUE(writer.ConsumeBytes(1));
  EXPECT_EQ(writer.RemainingBitCount(), 24u);

  EXPECT_TRUE(writer.ConsumeBits(4));
  EXPECT_EQ(writer.RemainingBitCount(), 20u);

  EXPECT_FALSE(writer.ConsumeBytes(3));  // Only 2.5 bytes left
}

TEST(BitBufferWriterTest, WriteUInt8_16_32) {
  std::array<uint8_t, 7> bytes = {0};
  BitBufferWriter writer(bytes.data(), 7);

  EXPECT_TRUE(writer.WriteUInt8(0x12));
  EXPECT_TRUE(writer.WriteUInt16(0x3456));
  EXPECT_TRUE(writer.WriteUInt32(0x789ABCDE));

  EXPECT_EQ(bytes[0], 0x12);
  EXPECT_EQ(bytes[1], 0x34);
  EXPECT_EQ(bytes[2], 0x56);
  EXPECT_EQ(bytes[3], 0x78);
  EXPECT_EQ(bytes[4], 0x9A);
  EXPECT_EQ(bytes[5], 0xBC);
  EXPECT_EQ(bytes[6], 0xDE);
}

TEST(BitBufferWriterTest, WriteExponentialGolomb) {
  std::array<uint8_t, 2> bytes = {0};
  BitBufferWriter writer(bytes.data(), 2);

  // val = 0 -> x = 1 (width 1). Code: 1
  EXPECT_TRUE(writer.WriteExponentialGolomb(0));
  // val = 1 -> x = 2 (width 2). Code: 010
  EXPECT_TRUE(writer.WriteExponentialGolomb(1));

  // Checking remaining capacity (16 - 1 - 3 = 12)
  EXPECT_EQ(writer.RemainingBitCount(), 12u);
}

}  // namespace
}  // namespace base
}  // namespace ave
