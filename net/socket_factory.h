/*
 * socket_factory.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_SOCKET_FACTORY_H
#define BASE_NET_SOCKET_FACTORY_H

#include "base/net/socket.h"

namespace ave {
namespace base {
namespace net {

class SocketFactory {
 public:
  virtual ~SocketFactory() {}

  // Returns a new socket.  The type can be SOCK_DGRAM and SOCK_STREAM.
  virtual Socket* CreateSocket(int family, int type) = 0;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_SOCKET_FACTORY_H */
