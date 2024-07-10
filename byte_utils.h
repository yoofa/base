/*
 * byte_utils.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BYTE_UTILS_H
#define BYTE_UTILS_H

#include <arpa/inet.h>

#include "base/types.h"

namespace ave {

constexpr int FOURCC(unsigned char c1,
                     unsigned char c2,
                     unsigned char c3,
                     unsigned char c4) {
  return ((c1) << 24 | (c2) << 16 | (c3) << 8 | (c4));
}

// NOLINTBEGIN(modernize-avoid-c-arrays)
template <size_t N>
constexpr int32_t FOURCC(const char (&s)[N]) {
  static_assert(N == 5, "fourcc: wrong length");
  return static_cast<unsigned char>(s[0]) << 24 | static_cast<unsigned char>(s[1]) << 16 |
         static_cast<unsigned char>(s[2]) << 8 | static_cast<unsigned char>(s[3]) << 0;
}
// NOLINTEND(modernize-avoid-c-arrays)

uint16_t U16_AT(const uint8_t* ptr);
uint32_t U32_AT(const uint8_t* ptr);
uint64_t U64_AT(const uint8_t* ptr);

uint16_t U16LE_AT(const uint8_t* ptr);
uint32_t U32LE_AT(const uint8_t* ptr);
uint64_t U64LE_AT(const uint8_t* ptr);

uint64_t ntoh64(uint64_t x);
uint64_t hton64(uint64_t x);

void MakeFourCCString(uint32_t x, char* s);

inline uint16_t HostToNetwork16(uint16_t n) {
  return htobe16(n);
}

inline uint32_t HostToNetwork32(uint32_t n) {
  return static_cast<uint32_t>(htobe32(n));
}

inline uint64_t HostToNetwork64(uint64_t n) {
  return htobe64(n);
}

inline uint16_t NetworkToHost16(uint16_t n) {
  return be16toh(n);
}

inline uint32_t NetworkToHost32(uint32_t n) {
  return static_cast<uint32_t>(be32toh(n));
}

inline uint64_t NetworkToHost64(uint64_t n) {
  return be64toh(n);
}
} /* namespace ave */

#endif /* !BYTE_UTILS_H */
