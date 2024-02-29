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
  SocketAddress(const IPAddress& ip, uint16_t port);
  SocketAddress(const uint32_t ip_as_host_order_integer, uint16_t port);
  SocketAddress(const SocketAddress& other);
  SocketAddress& operator=(const SocketAddress& other);

  void Clear();

  // Determines if this is a nil address (empty hostname, any IP, null port)
  bool IsNil() const;

  // Returns true if ip and port are set.
  bool IsComplete() const;

  void SetIP(const std::string& hostname);
  void SetIP(const IPAddress& ip);
  void SetIP(const uint32_t ip_as_host_order_integer);

  // Sets the IP address while retaining the hostname.  Useful for bypassing
  // DNS for a pre-resolved IP.
  // IP is given as an integer in host byte order. V4 only, to be deprecated.
  void SetResolvedIP(uint32_t ip_as_host_order_integer);

  // Sets the IP address while retaining the hostname.  Useful for bypassing
  // DNS for a pre-resolved IP.
  void SetResolvedIP(const IPAddress& ip);

  void SetPort(uint16_t port);

  const std::string& hostname() const;
  const IPAddress& ipaddr() const;
  uint32_t ip() const;
  int family() const;
  uint16_t port() const;

  // Returns the scope ID associated with this address. Scope IDs are a
  // necessary addition to IPv6 link-local addresses, with different network
  // interfaces having different scope-ids for their link-local addresses.
  // IPv4 address do not have scope_ids and sockaddr_in structures do not have
  // a field for them.
  int scope_id() const { return scope_id_; }
  void SetScopeID(int id) { scope_id_ = id; }

  std::string ToString() const;

  bool FromString(const std::string& address);

  bool IsAnyIP() const;
  bool IsLoopbackIP() const;

  bool operator==(const SocketAddress& other) const;

 private:
  std::string HostOrIPAsString() const;
  std::string hostname_;
  IPAddress ip_;
  uint16_t port_;
  int scope_id_;
  bool literal_;  // Indicates that 'hostname_' contains a literal IP string.
};

}  // namespace net
}  // namespace base
}  // namespace avp

#endif /* !AVP_BASE_NET_SOCKET_ADDRESS_H */
