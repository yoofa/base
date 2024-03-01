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

  // Returns the hostname.
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

  // Returns the 'host' portion of the address (hostname or IP) in a form
  // suitable for use in a URI. If both IP and hostname are present, hostname
  // is preferred. IPv6 addresses are enclosed in square brackets ('[' and ']').
  std::string HostAsURIString() const;

  // Same as HostAsURIString but anonymizes IP addresses by hiding the last
  // part.
  std::string HostAsSensitiveURIString() const;

  // Returns the port as a string.
  std::string PortAsString() const;

  // Returns hostname:port or [hostname]:port.
  std::string ToString() const;

  // Same as ToString but anonymizes it by hiding the last part.
  std::string ToSensitiveString() const;

  // Returns sensitive description of address in a form which both includes
  // resolved and unresolved addresses based on their availability.
  std::string ToSensitiveNameAndAddressString() const;

  // Parses hostname:port and [hostname]:port.
  bool FromString(const std::string& address);

  // Determines whether this represents a missing / any IP address.
  // That is, 0.0.0.0 or ::.
  // Hostname and/or port may be set.
  bool IsAnyIP() const;

  // Determines whether the IP address refers to a loopback address.
  // For v4 addresses this means the address is in the range 127.0.0.0/8.
  // For v6 addresses this means the address is ::1.
  bool IsLoopbackIP() const;

  // Determines whether the IP address is in one of the private ranges:
  // For v4: 127.0.0.0/8 10.0.0.0/8 192.168.0.0/16 172.16.0.0/12.
  // For v6: FE80::/16 and ::1.
  bool IsPrivateIP() const;

  // Determines whether the hostname has been resolved to an IP.
  bool IsUnresolvedIP() const;

  // Determines whether this address is identical to the given one.
  bool operator==(const SocketAddress& other) const;
  inline bool operator!=(const SocketAddress& addr) const {
    return !this->operator==(addr);
  }

  // Compares based on IP and then port.
  bool operator<(const SocketAddress& addr) const;

  // Determines whether this address has the same IP as the one given.
  bool EqualIPs(const SocketAddress& addr) const;

  // Determines whether this address has the same port as the one given.
  bool EqualPorts(const SocketAddress& addr) const;

  // Hashes this address into a small number.
  size_t Hash() const;

  // Write this address to a sockaddr_in.
  // If IPv6, will zero out the sockaddr_in and sets family to AF_UNSPEC.
  void ToSockAddr(sockaddr_in* saddr) const;

  // Read this address from a sockaddr_in.
  bool FromSockAddr(const sockaddr_in& saddr);

 private:
  std::string hostname_;
  IPAddress ip_;
  uint16_t port_;
  int scope_id_;
  bool literal_;  // Indicates that 'hostname_' contains a literal IP string.
};

SocketAddress EmptySocketAddressWithFamily(int family);

}  // namespace net
}  // namespace base
}  // namespace avp

#endif /* !AVP_BASE_NET_SOCKET_ADDRESS_H */
