/*
 * socket_address.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVP_BASE_NET_SOCKET_ADDRESS_H
#define AVP_BASE_NET_SOCKET_ADDRESS_H

#include <string>

#include "base/net/ip_address.h"

namespace avp {
namespace base {
namespace net {

class SocketAddress {
 public:
  SocketAddress();
  SocketAddress(const std::string& hostname, uint16_t port);
  SocketAddress(const IpAddress& ip, uint16_t port);
  SocketAddress(const uint32_t ip_as_host_order_integer, uint16_t port);
  SocketAddress(const SocketAddress& other);
  SocketAddress& operator=(const SocketAddress& other);

  void Clear();
  void SetIP(const std::string& hostname);
  void SetIP(const IpAddress& ip);
  void SetIP(const uint32_t ip_as_host_order_integer);
  void SetPort(uint16_t port);

  const std::string& hostname() const;
  const IpAddress& ipaddr() const;
  uint32_t ip() const;
  int family() const;
  uint16_t port() const;

  std::string ToString() const;

  bool FromString(const std::string& address);

  bool IsAnyIP() const;
  bool IsLoopbackIP() const;

  bool operator==(const SocketAddress& other) const;

 private:
  std::string HostOrIpAsString() const;
  std::string hostname_;
  IpAddress ip_;
  uint16_t port_;
};

}  // namespace net
}  // namespace base
}  // namespace avp

#endif /* !AVP_BASE_NET_SOCKET_ADDRESS_H */
