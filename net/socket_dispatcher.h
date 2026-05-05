/*
 * socket_dispatcher.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_SOCKET_DISPATCHER_H
#define BASE_NET_SOCKET_DISPATCHER_H

#include <cstdint>

#include "base/net/dispatcher.h"
#include "base/net/physical_socket.h"

namespace ave {
namespace base {
namespace net {

// SocketDispatcher combines PhysicalSocket and Dispatcher.
// It bridges the socket I/O with the event loop (SocketServer).
class SocketDispatcher : public PhysicalSocket, public Dispatcher {
 public:
  SocketDispatcher(PhysicalSocketServer* ss, int32_t family, int32_t type);
  ~SocketDispatcher() override;

  // Disallow copy
  SocketDispatcher(const SocketDispatcher&) = delete;
  SocketDispatcher& operator=(const SocketDispatcher&) = delete;

  // Socket interface overrides
  int32_t Bind(const SocketAddress& addr) override;
  int32_t Connect(const SocketAddress& addr) override;
  int32_t Listen(int32_t backlog) override;
  Socket* Accept(SocketAddress* paddr) override;
  int32_t Close() override;

  // Dispatcher interface
  int32_t GetDescriptor() override;
  bool IsDescriptorClosed() override;
  uint32_t GetRequestedEvents() override;
  void OnEvent(uint32_t events, int32_t error) override;

 protected:
  // For creating accepted sockets
  SocketDispatcher(PhysicalSocketServer* ss,
                   int32_t socket_fd,
                   int32_t family,
                   int32_t type);

 private:
  // Register/unregister with SocketServer
  void MaybeAddToServer();
  void RemoveFromServer();

  bool registered_;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_SOCKET_DISPATCHER_H */
