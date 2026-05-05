/*
 * dispatcher.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_DISPATCHER_H
#define BASE_NET_DISPATCHER_H

#include <cstdint>

namespace ave {
namespace base {
namespace net {

// Event flags for I/O multiplexing
enum DispatcherEvent : uint32_t {
  DE_READ = 0x0001,
  DE_WRITE = 0x0002,
  DE_CONNECT = 0x0004,
  DE_CLOSE = 0x0008,
  DE_ACCEPT = 0x0010,
};

// Dispatcher is an abstract interface for objects that can be monitored
// for I/O events by a SocketServer. It follows the Reactor pattern.
class Dispatcher {
 public:
  virtual ~Dispatcher() = default;

  // Returns the file descriptor to be monitored.
  virtual int32_t GetDescriptor() = 0;

  // Returns true if the dispatcher is still valid and should be monitored.
  virtual bool IsDescriptorClosed() = 0;

  // Returns the events this dispatcher is interested in (DE_READ, DE_WRITE,
  // etc.)
  virtual uint32_t GetRequestedEvents() = 0;

  // Called by SocketServer when events occur on the descriptor.
  virtual void OnEvent(uint32_t events, int32_t error) = 0;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_DISPATCHER_H */
