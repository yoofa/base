/*
 * ip_address.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "ip_address.h"

#include <arpa/inet.h>
#include <cstring>

namespace avp {
namespace base {
namespace net {

IpAddress::IpAddress() : family_(AF_UNSPEC) {
  memset(&address_, 0, sizeof(address_));
}

IpAddress::IpAddress(const in_addr& ipv4) : family_(AF_INET) {
  memset(&address_, 0, sizeof(address_));
  address_.ipv4_ = ipv4;
}

IpAddress::IpAddress(const in6_addr& ipv6) : family_(AF_INET6) {
  memset(&address_, 0, sizeof(address_));
  address_.ipv6_ = ipv6;
}

IpAddress::IpAddress(uint32_t ip_in_host_byte_order) : family_(AF_INET) {
  memset(&address_, 0, sizeof(address_));
  address_.ipv4_.s_addr = htobe32(ip_in_host_byte_order);
}

IpAddress::~IpAddress() {}

const IpAddress& IpAddress::operator=(const IpAddress& rhs) {
  if (this != &rhs) {
    family_ = rhs.family_;
    ::memcpy(&address_, &rhs.address_, sizeof(address_));
  }
  return *this;
}

bool IpAddress::operator==(const IpAddress& rhs) const {
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

bool IpAddress::operator!=(const IpAddress& rhs) const {
  return !(*this == rhs);
}

int IpAddress::family() const {
  return family_;
}

in_addr IpAddress::ipv4() const {
  return address_.ipv4_;
}

in6_addr IpAddress::ipv6() const {
  return address_.ipv6_;
}

std::string IpAddress::ToString() const {
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

bool IpFromAddressInfo(const struct addrinfo* info, IpAddress* out) {
  if (info == nullptr || info->ai_addr == nullptr || out == nullptr) {
    return false;
  }

  if (info->ai_addr->sa_family == AF_INET) {
    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(info->ai_addr);
    *out = IpAddress(addr->sin_addr);
    return true;
  } else if (info->ai_addr->sa_family == AF_INET6) {
    sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(info->ai_addr);
    *out = IpAddress(addr->sin6_addr);
    return true;
  } else {
    return false;
  }
}

bool IpFromString(const std::string& str, IpAddress* out) {
  if (out == nullptr) {
    return false;
  }
  in_addr addr;
  if (inet_pton(AF_INET, str.c_str(), &addr) == 1) {
    *out = IpAddress(addr);
    return true;
  } else {
    in6_addr addr6;
    if (inet_pton(AF_INET6, str.c_str(), &addr6) == 1) {
      *out = IpAddress(addr6);
      return true;
    }
  }
  return false;
}

bool IpIsAny(const IpAddress& ip) {
  if (ip.family() == AF_INET) {
    return ip == IpAddress(INADDR_ANY);

  } else if (ip.family() == AF_INET6) {
    return ip == IpAddress(in6addr_any);
  }
  return false;
}

bool IpIsLoopback(const IpAddress& ip) {
  if (ip.family() == AF_INET) {
    return ip == IpAddress(INADDR_LOOPBACK);

  } else if (ip.family() == AF_INET6) {
    return ip == IpAddress(in6addr_loopback);
  }
  return false;
}

IpAddress GetLoopbackAddress(int family) {
  if (family == AF_INET) {
    return IpAddress(INADDR_LOOPBACK);
  } else if (family == AF_INET6) {
    return IpAddress(in6addr_loopback);
  }
  return IpAddress();
}

IpAddress GetAnyAddress(int family) {
  if (family == AF_INET) {
    return IpAddress(INADDR_ANY);
  } else if (family == AF_INET6) {
    return IpAddress(in6addr_any);
  }
  return IpAddress();
}

}  // namespace net
}  // namespace base
}  // namespace avp
