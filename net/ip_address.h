/*
 * ip_address.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVP_BASE_NET_ADDRESS_H
#define AVP_BASE_NET_ADDRESS_H

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

namespace avp {
namespace base {
namespace net {

class IPAddress {
 public:
  IPAddress();
  explicit IPAddress(const in_addr& ipv4);
  explicit IPAddress(const in6_addr& ipv6);
  explicit IPAddress(uint32_t ip_in_host_byte_order);
  virtual ~IPAddress();
  const IPAddress& operator=(const IPAddress& rhs);

  bool operator==(const IPAddress& rhs) const;
  bool operator!=(const IPAddress& rhs) const;
  bool operator<(const IPAddress& other) const;
  bool operator>(const IPAddress& other) const;

  int family() const;
  in_addr ipv4() const;
  in6_addr ipv6() const;

  // Returns the number of bytes needed to store the raw address.
  size_t Size() const;

  // Wraps inet_ntop.
  std::string ToString() const;
  // Same as ToString but anonymizes it by hiding the last part.
  std::string ToSensitiveString() const;

  // Returns an unmapped address from a possibly-mapped address.
  // Returns the same address if this isn't a mapped address.
  IPAddress Normalized() const;

  // Returns this address as an IPv6 address.
  // Maps v4 addresses (as ::ffff:a.b.c.d), returns v6 addresses unchanged.
  IPAddress AsIPv6Address() const;

  // For socketaddress' benefit. Returns the IP in host byte order.
  uint32_t v4AddressAsHostOrderInteger() const;

  // Get the network layer overhead per packet based on the IP address family.
  int overhead() const;

  // Whether this is an unspecified IP address.
  bool IsNil() const;

 private:
  int family_;
  union {
    struct in_addr ipv4_;
    struct in6_addr ipv6_;
  } address_;
};

bool IPFromAddressInfo(const struct addrinfo* info, IPAddress* out);

bool IPFromString(const std::string& str, IPAddress* out);

bool IPIsAny(const IPAddress& ip);
bool IPIsLoopback(const IPAddress& ip);
bool IPIsLinkLocal(const IPAddress& ip);
// Identify a private network address like "192.168.111.222"
// (see https://en.wikipedia.org/wiki/Private_network )
bool IPIsPrivateNetwork(const IPAddress& ip);
// Identify a shared network address like "100.72.16.122"
// (see RFC6598)
bool IPIsSharedNetwork(const IPAddress& ip);
// Identify if an IP is "private", that is a loopback
// or an address belonging to a link-local, a private network or a shared
// network.
bool IPIsPrivate(const IPAddress& ip);
bool IPIsUnspec(const IPAddress& ip);
size_t HashIP(const IPAddress& ip);

// These are only really applicable for IPv6 addresses.
bool IPIs6Bone(const IPAddress& ip);
bool IPIs6To4(const IPAddress& ip);
bool IPIsMacBased(const IPAddress& ip);
bool IPIsSiteLocal(const IPAddress& ip);
bool IPIsTeredo(const IPAddress& ip);
bool IPIsULA(const IPAddress& ip);
bool IPIsV4Compatibility(const IPAddress& ip);
bool IPIsV4Mapped(const IPAddress& ip);

// Returns the precedence value for this IP as given in RFC3484.
int IPAddressPrecedence(const IPAddress& ip);

// Returns 'ip' truncated to be 'length' bits long.
IPAddress TruncateIP(const IPAddress& ip, int length);

IPAddress GetLoopbackAddress(int family);
IPAddress GetAnyAddress(int family);

// Returns the number of contiguously set bits, counting from the MSB in network
// byte order, in this IPAddress. Bits after the first 0 encountered are not
// counted.
int CountIPMaskBits(const IPAddress& mask);

}  // namespace net

}  // namespace base
}  // namespace avp

#endif /* !AVP_BASE_NET_ADDRESS_H */
