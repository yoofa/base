/*
 * socket_address.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "socket_address.h"

#include <cstdlib>
#include <sstream>
#include "base/net/ip_address.h"

#include <base/byte_utils.h>

namespace ave {
namespace base {
namespace net {

SocketAddress::SocketAddress() {
  Clear();
}

SocketAddress::SocketAddress(const std::string& hostname, uint16_t port) {
  SetIP(hostname);
  SetPort(port);
}

SocketAddress::SocketAddress(const IPAddress& ip, uint16_t port) {
  SetIP(ip);
  SetPort(port);
}

SocketAddress::SocketAddress(const uint32_t ip_as_host_order_integer,
                             uint16_t port) {
  SetIP(ip_as_host_order_integer);
  SetPort(port);
}

SocketAddress::SocketAddress(const SocketAddress& other) {
  hostname_ = other.hostname_;
  ip_ = other.ip_;
  port_ = other.port_;
  scope_id_ = other.scope_id_;
  literal_ = other.literal_;
}

SocketAddress& SocketAddress::operator=(const SocketAddress& other) {
  hostname_ = other.hostname_;
  ip_ = other.ip_;
  port_ = other.port_;
  scope_id_ = other.scope_id_;
  literal_ = other.literal_;
  return *this;
}

void SocketAddress::Clear() {
  hostname_.clear();
  ip_ = IPAddress();
  port_ = 0;
  scope_id_ = 0;
  literal_ = false;
}

bool SocketAddress::IsNil() const {
  return hostname_.empty() && IPIsUnspec(ip_) && 0 == port_;
}

bool SocketAddress::IsComplete() const {
  return (!IPIsAny(ip_)) && (0 != port_);
}

void SocketAddress::SetIP(const std::string& hostname) {
  hostname_ = hostname;
  literal_ = IPFromString(hostname, &ip_);
  if (!literal_) {
    ip_ = IPAddress();
  }
  scope_id_ = 0;
}

void SocketAddress::SetIP(const IPAddress& ip) {
  hostname_.clear();
  literal_ = false;
  ip_ = ip;
  scope_id_ = 0;
}

void SocketAddress::SetIP(const uint32_t ip_as_host_order_integer) {
  hostname_.clear();
  literal_ = false;
  ip_ = IPAddress(ip_as_host_order_integer);
  scope_id_ = 0;
}

void SocketAddress::SetResolvedIP(uint32_t ip_as_host_order_integer) {
  ip_ = IPAddress(ip_as_host_order_integer);
  scope_id_ = 0;
}

void SocketAddress::SetResolvedIP(const IPAddress& ip) {
  ip_ = ip;
  scope_id_ = 0;
}

void SocketAddress::SetPort(uint16_t port) {
  port_ = port;
}

const std::string& SocketAddress::hostname() const {
  return hostname_;
}

const IPAddress& SocketAddress::ipaddr() const {
  return ip_;
}

uint32_t SocketAddress::ip() const {
  if (ip_.family() == AF_INET) {
    return ip_.ipv4().s_addr;
  }
  return 0;
}

int SocketAddress::family() const {
  return ip_.family();
}

uint16_t SocketAddress::port() const {
  return port_;
}

std::string SocketAddress::HostAsURIString() const {
  // If the hostname was a literal IP string, it may need to have square
  // brackets added (for SocketAddress::ToString()).
  if (!literal_ && !hostname_.empty())
    return hostname_;
  if (ip_.family() == AF_INET6) {
    return "[" + ip_.ToString() + "]";
  } else {
    return ip_.ToString();
  }
}

std::string SocketAddress::HostAsSensitiveURIString() const {
  // If the hostname was a literal IP string, it may need to have square
  // brackets added (for SocketAddress::ToString()).
  if (!literal_ && !hostname_.empty())
    return hostname_;
  if (ip_.family() == AF_INET6) {
    return "[" + ip_.ToSensitiveString() + "]";
  } else {
    return ip_.ToSensitiveString();
  }
}

std::string SocketAddress::PortAsString() const {
  return std::to_string(port_);
}

std::string SocketAddress::ToString() const {
  std::stringstream ss;
  ss << HostAsURIString() << ":" << port_;
  return ss.str();
}

std::string SocketAddress::ToSensitiveString() const {
  std::stringstream ss;
  ss << HostAsSensitiveURIString() << ":" << port();
  return ss.str();
}

std::string SocketAddress::ToSensitiveNameAndAddressString() const {
  if (IsUnresolvedIP() || literal_ || hostname_.empty()) {
    return ToSensitiveString();
  }
  std::stringstream ss;
  ss << HostAsSensitiveURIString() << ":" << port();
  ss << " (";
  if (ip_.family() == AF_INET6) {
    ss << "[" << ipaddr().ToSensitiveString() << "]";
  } else {
    ss << ipaddr().ToSensitiveString();
  }
  ss << ":" << port() << ")";

  return ss.str();
}

bool SocketAddress::FromString(const std::string& address) {
  // [ipv6]:port
  if (address[0] == '[') {
    // find the last ']'
    size_t pos = address.rfind(']');
    if (pos == std::string::npos) {
      return false;
    }
    size_t port_pos = address.find(':', pos);
    if (port_pos == std::string::npos) {
      return false;
    }
    SetIP(address.substr(1, pos - 1));
    SetPort(strtol(address.substr(port_pos + 1).c_str(), NULL, 10));
  } else {
    // ipv4/host:port
    size_t pos = address.rfind(':');
    if (pos == std::string::npos) {
      return false;
    }
    SetIP(address.substr(0, pos));
    SetPort(strtol(address.substr(pos + 1).c_str(), NULL, 10));
  }
  return true;
}

bool SocketAddress::IsAnyIP() const {
  return IPIsAny(ip_);
}

bool SocketAddress::IsLoopbackIP() const {
  return IPIsLoopback(ip_) ||
         (IPIsAny(ip_) && 0 == strcmp(hostname_.c_str(), "localhost"));
}

bool SocketAddress::IsPrivateIP() const {
  return IPIsPrivate(ip_);
}

bool SocketAddress::IsUnresolvedIP() const {
  return IPIsUnspec(ip_) && !literal_ && !hostname_.empty();
}

bool SocketAddress::operator==(const SocketAddress& addr) const {
  return EqualIPs(addr) && EqualPorts(addr);
}

bool SocketAddress::operator<(const SocketAddress& addr) const {
  if (ip_ != addr.ip_)
    return ip_ < addr.ip_;

  // We only check hostnames if both IPs are ANY or unspecified.  This matches
  // EqualIPs().
  if ((IPIsAny(ip_) || IPIsUnspec(ip_)) && hostname_ != addr.hostname_)
    return hostname_ < addr.hostname_;

  return port_ < addr.port_;
}

bool SocketAddress::EqualIPs(const SocketAddress& addr) const {
  return (ip_ == addr.ip_) &&
         ((!IPIsAny(ip_) && !IPIsUnspec(ip_)) || (hostname_ == addr.hostname_));
}

bool SocketAddress::EqualPorts(const SocketAddress& addr) const {
  return (port_ == addr.port_);
}

size_t SocketAddress::Hash() const {
  size_t h = 0;
  h ^= HashIP(ip_);
  h ^= port_ | (port_ << 16);
  return h;
}

void SocketAddress::ToSockAddr(sockaddr_in* saddr) const {
  memset(saddr, 0, sizeof(*saddr));
  if (ip_.family() != AF_INET) {
    saddr->sin_family = AF_UNSPEC;
    return;
  }
  saddr->sin_family = AF_INET;
  saddr->sin_port = HostToNetwork16(port_);
  if (IPIsAny(ip_)) {
    saddr->sin_addr.s_addr = INADDR_ANY;
  } else {
    saddr->sin_addr = ip_.ipv4();
  }
}

bool SocketAddress::FromSockAddr(const sockaddr_in& saddr) {
  if (saddr.sin_family != AF_INET)
    return false;
  SetIP(NetworkToHost32(saddr.sin_addr.s_addr));
  SetPort(NetworkToHost16(saddr.sin_port));
  literal_ = false;
  return true;
}

SocketAddress EmptySocketAddressWithFamily(int family) {
  if (family == AF_INET) {
    return SocketAddress(IPAddress(INADDR_ANY), 0);
  } else if (family == AF_INET6) {
    return SocketAddress(IPAddress(in6addr_any), 0);
  }
  return SocketAddress();
}

}  // namespace net
}  // namespace base
}  // namespace ave
