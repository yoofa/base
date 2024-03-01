/*
 * ip_address.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "ip_address.h"

#include <arpa/inet.h>
#include <cstring>

#include <base/byte_utils.h>

namespace avp {
namespace base {
namespace net {

// Prefixes used for categorizing IPv6 addresses.
static const in6_addr kV4MappedPrefix = {
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0}}};
static const in6_addr k6To4Prefix = {{{0x20, 0x02, 0}}};
static const in6_addr kTeredoPrefix = {{{0x20, 0x01, 0x00, 0x00}}};
static const in6_addr kV4CompatibilityPrefix = {{{0}}};
static const in6_addr k6BonePrefix = {{{0x3f, 0xfe, 0}}};
static const in6_addr kPrivateNetworkPrefix = {{{0xFD}}};

IPAddress::IPAddress() : family_(AF_UNSPEC) {
  memset(&address_, 0, sizeof(address_));
}

IPAddress::IPAddress(const in_addr& ipv4) : family_(AF_INET) {
  memset(&address_, 0, sizeof(address_));
  address_.ipv4_ = ipv4;
}

IPAddress::IPAddress(const in6_addr& ipv6) : family_(AF_INET6) {
  memset(&address_, 0, sizeof(address_));
  address_.ipv6_ = ipv6;
}

IPAddress::IPAddress(uint32_t ip_in_host_byte_order) : family_(AF_INET) {
  memset(&address_, 0, sizeof(address_));
  address_.ipv4_.s_addr = htobe32(ip_in_host_byte_order);
}

IPAddress::~IPAddress() {}

const IPAddress& IPAddress::operator=(const IPAddress& rhs) {
  if (this != &rhs) {
    family_ = rhs.family_;
    ::memcpy(&address_, &rhs.address_, sizeof(address_));
  }
  return *this;
}

bool IPAddress::operator==(const IPAddress& rhs) const {
  if (family_ != rhs.family_) {
    return false;
  }
  if (family_ == AF_INET) {
    return ::memcmp(&address_.ipv4_, &rhs.address_.ipv4_, sizeof(in_addr)) == 0;
  } else if (family_ == AF_INET6) {
    return ::memcmp(&address_.ipv6_, &rhs.address_.ipv6_, sizeof(in6_addr)) ==
           0;
  }

  return family_ == AF_UNSPEC;
}

bool IPAddress::operator!=(const IPAddress& rhs) const {
  return !(*this == rhs);
}

bool IPAddress::operator>(const IPAddress& other) const {
  return (*this) != other && !((*this) < other);
}

bool IPAddress::operator<(const IPAddress& other) const {
  // IPv4 is 'less than' IPv6
  if (family_ != other.family_) {
    if (family_ == AF_UNSPEC) {
      return true;
    }
    if (family_ == AF_INET && other.family_ == AF_INET6) {
      return true;
    }
    return false;
  }
  // Comparing addresses of the same family.
  switch (family_) {
    case AF_INET: {
      return NetworkToHost32(address_.ipv4_.s_addr) <
             NetworkToHost32(other.address_.ipv4_.s_addr);
    }
    case AF_INET6: {
      return memcmp(&address_.ipv6_.s6_addr, &other.address_.ipv6_.s6_addr,
                    16) < 0;
    }
  }
  // Catches AF_UNSPEC and invalid addresses.
  return false;
}

int IPAddress::family() const {
  return family_;
}

in_addr IPAddress::ipv4() const {
  return address_.ipv4_;
}

in6_addr IPAddress::ipv6() const {
  return address_.ipv6_;
}

size_t IPAddress::Size() const {
  switch (family_) {
    case AF_INET:
      return sizeof(in_addr);
    case AF_INET6:
      return sizeof(in6_addr);
  }
  return 0;
}

std::string IPAddress::ToString() const {
  if (family_ != AF_INET && family_ != AF_INET6) {
    return std::string();
  }

  char buf[INET6_ADDRSTRLEN];
  const void* src = &address_.ipv4_;
  if (family_ == AF_INET6) {
    src = &address_.ipv6_;
  }

  if (inet_ntop(family_, src, buf, sizeof(buf)) == 0) {
    return std::string();
  }

  return std::string(buf);
}

std::string IPAddress::ToSensitiveString() const {
  switch (family_) {
    case AF_INET: {
      std::string address = ToString();
      size_t find_pos = address.rfind('.');
      if (find_pos == std::string::npos)
        return std::string();
      address.resize(find_pos);
      address += ".x";
      return address;
    }
    case AF_INET6: {
      std::string result;
      result.resize(INET6_ADDRSTRLEN);
      in6_addr addr = ipv6();
      size_t len = snprintf(&(result[0]), result.size(), "%x:%x:%x:x:x:x:x:x",
                            (addr.s6_addr[0] << 8) + addr.s6_addr[1],
                            (addr.s6_addr[2] << 8) + addr.s6_addr[3],
                            (addr.s6_addr[4] << 8) + addr.s6_addr[5]);
      result.resize(len);
      return result;
    }
  }
  return std::string();
}

static in_addr ExtractMappedAddress(const in6_addr& in6) {
  in_addr ipv4;
  ::memcpy(&ipv4.s_addr, &in6.s6_addr[12], sizeof(ipv4.s_addr));
  return ipv4;
}

IPAddress IPAddress::Normalized() const {
  if (family_ != AF_INET6) {
    return *this;
  }
  if (!IPIsV4Mapped(*this)) {
    return *this;
  }
  in_addr addr = ExtractMappedAddress(address_.ipv6_);
  return IPAddress(addr);
}

IPAddress IPAddress::AsIPv6Address() const {
  if (family_ != AF_INET) {
    return *this;
  }
  in6_addr v6addr = kV4MappedPrefix;
  ::memcpy(&v6addr.s6_addr[12], &address_.ipv4_.s_addr,
           sizeof(address_.ipv4_.s_addr));
  return IPAddress(v6addr);
}

uint32_t IPAddress::v4AddressAsHostOrderInteger() const {
  if (family_ == AF_INET) {
    return NetworkToHost32(address_.ipv4_.s_addr);
  } else {
    return 0;
  }
}

int IPAddress::overhead() const {
  switch (family_) {
    case AF_INET:  // IPv4
      return 20;
    case AF_INET6:  // IPv6
      return 40;
    default:
      return 0;
  }
}

bool IPAddress::IsNil() const {
  return IPIsUnspec(*this);
}

///////////////////////////////////////////////////////////////////////////////

bool IPFromAddressInfo(const struct addrinfo* info, IPAddress* out) {
  if (info == nullptr || info->ai_addr == nullptr || out == nullptr) {
    return false;
  }

  if (info->ai_addr->sa_family == AF_INET) {
    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(info->ai_addr);
    *out = IPAddress(addr->sin_addr);
    return true;
  } else if (info->ai_addr->sa_family == AF_INET6) {
    sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(info->ai_addr);
    *out = IPAddress(addr->sin6_addr);
    return true;
  } else {
    return false;
  }
}

bool IPFromString(const std::string& str, IPAddress* out) {
  if (out == nullptr) {
    return false;
  }
  in_addr addr;
  if (inet_pton(AF_INET, str.c_str(), &addr) == 1) {
    *out = IPAddress(addr);
    return true;
  } else {
    in6_addr addr6;
    if (inet_pton(AF_INET6, str.c_str(), &addr6) == 1) {
      *out = IPAddress(addr6);
      return true;
    }
  }
  return false;
}

bool IPIsAny(const IPAddress& ip) {
  if (ip.family() == AF_INET) {
    return ip == IPAddress(INADDR_ANY);

  } else if (ip.family() == AF_INET6) {
    return ip == IPAddress(in6addr_any);
  }
  return false;
}

bool IPIsLoopback(const IPAddress& ip) {
  if (ip.family() == AF_INET) {
    return ip == IPAddress(INADDR_LOOPBACK);

  } else if (ip.family() == AF_INET6) {
    return ip == IPAddress(in6addr_loopback);
  }
  return false;
}

static bool IPIsLinkLocalV4(const IPAddress& ip) {
  uint32_t ip_in_host_order = ip.v4AddressAsHostOrderInteger();
  return ((ip_in_host_order >> 16) == ((169 << 8) | 254));
}

static bool IPIsLinkLocalV6(const IPAddress& ip) {
  // Can't use the helper because the prefix is 10 bits.
  in6_addr addr = ip.ipv6();
  return (addr.s6_addr[0] == 0xFE) && ((addr.s6_addr[1] & 0xC0) == 0x80);
}

bool IPIsLinkLocal(const IPAddress& ip) {
  switch (ip.family()) {
    case AF_INET: {
      return IPIsLinkLocalV4(ip);
    }
    case AF_INET6: {
      return IPIsLinkLocalV6(ip);
    }
  }
  return false;
}

static bool IPIsPrivateNetworkV4(const IPAddress& ip) {
  uint32_t ip_in_host_order = ip.v4AddressAsHostOrderInteger();
  return ((ip_in_host_order >> 24) == 10) ||
         ((ip_in_host_order >> 20) == ((172 << 4) | 1)) ||
         ((ip_in_host_order >> 16) == ((192 << 8) | 168));
}

static bool IPIsHelper(const IPAddress& ip,
                       const in6_addr& tomatch,
                       int length) {
  // Helper method for checking IP prefix matches (but only on whole byte
  // lengths). Length is in bits.
  in6_addr addr = ip.ipv6();
  return ::memcmp(&addr, &tomatch, (length >> 3)) == 0;
}

static bool IPIsPrivateNetworkV6(const IPAddress& ip) {
  return IPIsHelper(ip, kPrivateNetworkPrefix, 8);
}

bool IPIsPrivateNetwork(const IPAddress& ip) {
  switch (ip.family()) {
    case AF_INET: {
      return IPIsPrivateNetworkV4(ip);
    }
    case AF_INET6: {
      return IPIsPrivateNetworkV6(ip);
    }
  }
  return false;
}

static bool IPIsSharedNetworkV4(const IPAddress& ip) {
  uint32_t ip_in_host_order = ip.v4AddressAsHostOrderInteger();
  return (ip_in_host_order >> 22) == ((100 << 2) | 1);
}

bool IPIsSharedNetwork(const IPAddress& ip) {
  if (ip.family() == AF_INET) {
    return IPIsSharedNetworkV4(ip);
  }
  return false;
}

bool IPIsPrivate(const IPAddress& ip) {
  return IPIsLinkLocal(ip) || IPIsLoopback(ip) || IPIsPrivateNetwork(ip) ||
         IPIsSharedNetwork(ip);
}

bool IPIsUnspec(const IPAddress& ip) {
  return ip.family() == AF_UNSPEC;
}

size_t HashIP(const IPAddress& ip) {
  switch (ip.family()) {
    case AF_INET: {
      return ip.ipv4().s_addr;
    }
    case AF_INET6: {
      in6_addr v6addr = ip.ipv6();
      const uint32_t* v6_as_ints =
          reinterpret_cast<const uint32_t*>(&v6addr.s6_addr);
      return v6_as_ints[0] ^ v6_as_ints[1] ^ v6_as_ints[2] ^ v6_as_ints[3];
    }
  }
  return 0;
}

bool IPIs6Bone(const IPAddress& ip) {
  return IPIsHelper(ip, k6BonePrefix, 16);
}

bool IPIs6To4(const IPAddress& ip) {
  return IPIsHelper(ip, k6To4Prefix, 16);
}

// According to http://www.ietf.org/rfc/rfc2373.txt, Appendix A, page 19.  An
// address which contains MAC will have its 11th and 12th bytes as FF:FE as well
// as the U/L bit as 1.
bool IPIsMacBased(const IPAddress& ip) {
  in6_addr addr = ip.ipv6();
  return ((addr.s6_addr[8] & 0x02) && addr.s6_addr[11] == 0xFF &&
          addr.s6_addr[12] == 0xFE);
}

bool IPIsSiteLocal(const IPAddress& ip) {
  // Can't use the helper because the prefix is 10 bits.
  in6_addr addr = ip.ipv6();
  return addr.s6_addr[0] == 0xFE && (addr.s6_addr[1] & 0xC0) == 0xC0;
}

bool IPIsTeredo(const IPAddress& ip) {
  return IPIsHelper(ip, kTeredoPrefix, 32);
}

bool IPIsULA(const IPAddress& ip) {
  // Can't use the helper because the prefix is 7 bits.
  in6_addr addr = ip.ipv6();
  return (addr.s6_addr[0] & 0xFE) == 0xFC;
}

bool IPIsV4Compatibility(const IPAddress& ip) {
  return IPIsHelper(ip, kV4CompatibilityPrefix, 96);
}

bool IPIsV4Mapped(const IPAddress& ip) {
  return IPIsHelper(ip, kV4MappedPrefix, 96);
}

int IPAddressPrecedence(const IPAddress& ip) {
  // Precedence values from RFC 3484-bis. Prefers native v4 over 6to4/Teredo.
  if (ip.family() == AF_INET) {
    return 30;
  } else if (ip.family() == AF_INET6) {
    if (IPIsLoopback(ip)) {
      return 60;
    } else if (IPIsULA(ip)) {
      return 50;
    } else if (IPIsV4Mapped(ip)) {
      return 30;
    } else if (IPIs6To4(ip)) {
      return 20;
    } else if (IPIsTeredo(ip)) {
      return 10;
    } else if (IPIsV4Compatibility(ip) || IPIsSiteLocal(ip) || IPIs6Bone(ip)) {
      return 1;
    } else {
      // A 'normal' IPv6 address.
      return 40;
    }
  }
  return 0;
}

IPAddress TruncateIP(const IPAddress& ip, int length) {
  if (length < 0) {
    return IPAddress();
  }
  if (ip.family() == AF_INET) {
    if (length > 31) {
      return ip;
    }
    if (length == 0) {
      return IPAddress(INADDR_ANY);
    }
    int mask = (0xFFFFFFFF << (32 - length));
    uint32_t host_order_ip = NetworkToHost32(ip.ipv4().s_addr);
    in_addr masked;
    masked.s_addr = HostToNetwork32(host_order_ip & mask);
    return IPAddress(masked);
  } else if (ip.family() == AF_INET6) {
    if (length > 127) {
      return ip;
    }
    if (length == 0) {
      return IPAddress(in6addr_any);
    }
    in6_addr v6addr = ip.ipv6();
    int position = length / 32;
    int inner_length = 32 - (length - (position * 32));
    // Note: 64bit mask constant needed to allow possible 32-bit left shift.
    uint32_t inner_mask = 0xFFFFFFFFLL << inner_length;
    uint32_t* v6_as_ints = reinterpret_cast<uint32_t*>(&v6addr.s6_addr);
    for (int i = 0; i < 4; ++i) {
      if (i == position) {
        uint32_t host_order_inner = NetworkToHost32(v6_as_ints[i]);
        v6_as_ints[i] = HostToNetwork32(host_order_inner & inner_mask);
      } else if (i > position) {
        v6_as_ints[i] = 0;
      }
    }
    return IPAddress(v6addr);
  }
  return IPAddress();
}

IPAddress GetLoopbackAddress(int family) {
  if (family == AF_INET) {
    return IPAddress(INADDR_LOOPBACK);
  } else if (family == AF_INET6) {
    return IPAddress(in6addr_loopback);
  }
  return IPAddress();
}

IPAddress GetAnyAddress(int family) {
  if (family == AF_INET) {
    return IPAddress(INADDR_ANY);
  } else if (family == AF_INET6) {
    return IPAddress(in6addr_any);
  }
  return IPAddress();
}

int CountIPMaskBits(const IPAddress& mask) {
  uint32_t word_to_count = 0;
  int bits = 0;
  switch (mask.family()) {
    case AF_INET: {
      word_to_count = NetworkToHost32(mask.ipv4().s_addr);
      break;
    }
    case AF_INET6: {
      in6_addr v6addr = mask.ipv6();
      const uint32_t* v6_as_ints =
          reinterpret_cast<const uint32_t*>(&v6addr.s6_addr);
      int i = 0;
      for (; i < 4; ++i) {
        if (v6_as_ints[i] != 0xFFFFFFFF) {
          break;
        }
      }
      if (i < 4) {
        word_to_count = NetworkToHost32(v6_as_ints[i]);
      }
      bits = (i * 32);
      break;
    }
    default: {
      return 0;
    }
  }
  if (word_to_count == 0) {
    return bits;
  }

  // Public domain bit-twiddling hack from:
  // http://graphics.stanford.edu/~seander/bithacks.html
  // Counts the trailing 0s in the word.
  unsigned int zeroes = 32;
  // This could also be written word_to_count &= -word_to_count, but
  // MSVC emits warning C4146 when negating an unsigned number.
  word_to_count &= ~word_to_count + 1;  // Isolate lowest set bit.
  if (word_to_count)
    zeroes--;
  if (word_to_count & 0x0000FFFF)
    zeroes -= 16;
  if (word_to_count & 0x00FF00FF)
    zeroes -= 8;
  if (word_to_count & 0x0F0F0F0F)
    zeroes -= 4;
  if (word_to_count & 0x33333333)
    zeroes -= 2;
  if (word_to_count & 0x55555555)
    zeroes -= 1;

  return bits + (32 - zeroes);
}

}  // namespace net
}  // namespace base
}  // namespace avp
