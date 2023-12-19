/*
 * socket_address.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "socket_address.h"

#include <cstdlib>
#include <sstream>

namespace avp {
namespace base {
namespace net {

SocketAddress::SocketAddress() {
  Clear();
}

SocketAddress::SocketAddress(const std::string& hostname, uint16_t port) {
  SetIP(hostname);
  SetPort(port);
}

SocketAddress::SocketAddress(const IpAddress& ip, uint16_t port) {
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
}

SocketAddress& SocketAddress::operator=(const SocketAddress& other) {
  hostname_ = other.hostname_;
  ip_ = other.ip_;
  port_ = other.port_;
  return *this;
}

void SocketAddress::Clear() {
  hostname_.clear();
  ip_ = IpAddress();
  port_ = 0;
}

void SocketAddress::SetIP(const std::string& hostname) {
  hostname_ = hostname;
  if (IpFromString(hostname, &ip_) == false) {
    ip_ = IpAddress();
  }
}

void SocketAddress::SetIP(const IpAddress& ip) {
  hostname_.clear();
  ip_ = ip;
}

void SocketAddress::SetIP(const uint32_t ip_as_host_order_integer) {
  hostname_.clear();
  ip_ = IpAddress(ip_as_host_order_integer);
}

void SocketAddress::SetPort(uint16_t port) {
  port_ = port;
}

const std::string& SocketAddress::hostname() const {
  return hostname_;
}

const IpAddress& SocketAddress::ipaddr() const {
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

std::string SocketAddress::ToString() const {
  std::stringstream ss;
  ss << HostOrIpAsString() << ":" << port_;
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
  return IpIsAny(ip_);
}

bool SocketAddress::IsLoopbackIP() const {
  return IpIsLoopback(ip_);
}

bool SocketAddress::operator==(const SocketAddress& other) const {
  return hostname_ == other.hostname_ && ip_ == other.ip_ &&
         port_ == other.port_;
}

std::string SocketAddress::HostOrIpAsString() const {
  if (!hostname_.empty()) {
    return hostname_;
  }
  return ip_.ToString();
}

}  // namespace net
}  // namespace base
}  // namespace avp
