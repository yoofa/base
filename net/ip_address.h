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

class IpAddress {
 public:
  IpAddress();
  explicit IpAddress(const in_addr& ipv4);
  explicit IpAddress(const in6_addr& ipv6);
  explicit IpAddress(uint32_t ip_in_host_byte_order);
  virtual ~IpAddress();
  const IpAddress& operator=(const IpAddress& rhs);

  bool operator==(const IpAddress& rhs) const;
  bool operator!=(const IpAddress& rhs) const;

  int family() const;
  in_addr ipv4() const;
  in6_addr ipv6() const;

  std::string ToString() const;

 private:
  int family_;
  union {
    struct in_addr ipv4_;
    struct in6_addr ipv6_;
  } address_;
};

bool IpFromAddressInfo(const struct addrinfo* info, IpAddress* out);

bool IpFromString(const std::string& str, IpAddress* out);

bool IpIsAny(const IpAddress& ip);
bool IpIsLoopback(const IpAddress& ip);

IpAddress GetLoopbackAddress(int family);
IpAddress GetAnyAddress(int family);

}  // namespace net

}  // namespace base
}  // namespace avp

#endif /* !AVP_BASE_NET_ADDRESS_H */
