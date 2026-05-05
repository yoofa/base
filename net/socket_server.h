/*
 * socket_server.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_SOCKET_SERVER_H
#define BASE_NET_SOCKET_SERVER_H

#include <cstdint>

#include "base/net/dispatcher.h"
#include "base/net/socket_factory.h"

namespace ave {
namespace base {
namespace net {

// SocketServer is the event loop core that monitors file descriptors
// and dispatches I/O events. It follows the Reactor pattern.
class SocketServer : public SocketFactory {
 public:
  static constexpr int32_t kForever = -1;

  ~SocketServer() override = default;

  // Waits for I/O events or timeout.
  // cms: timeout in milliseconds, kForever means wait forever.
  // Returns true if events were processed, false on timeout or error.
  virtual bool Wait(int32_t cms) = 0;

  // Wakes up the Wait() call from another thread.
  virtual void WakeUp() = 0;

  // Registers a dispatcher to be monitored for events.
  virtual void Add(Dispatcher* dispatcher) = 0;

  // Unregisters a dispatcher.
  virtual void Remove(Dispatcher* dispatcher) = 0;

  // Updates the events a dispatcher is interested in.
  virtual void Update(Dispatcher* dispatcher) = 0;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_SOCKET_SERVER_H */
