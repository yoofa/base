/*
 * physical_socket_server.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_PHYSICAL_SOCKET_SERVER_H
#define BASE_NET_PHYSICAL_SOCKET_SERVER_H

#include <cstdint>
#include <mutex>
#include <set>
#include <vector>

#include "base/net/socket_server.h"

namespace ave {
namespace base {
namespace net {

// PhysicalSocketServer implements SocketServer using Linux epoll.
// It provides the event loop for monitoring socket I/O events.
class PhysicalSocketServer : public SocketServer {
 public:
  PhysicalSocketServer();
  ~PhysicalSocketServer() override;

  // Disallow copy
  PhysicalSocketServer(const PhysicalSocketServer&) = delete;
  PhysicalSocketServer& operator=(const PhysicalSocketServer&) = delete;

  // SocketFactory interface
  Socket* CreateSocket(int32_t family, int32_t type) override;

  // SocketServer interface
  bool Wait(int32_t cms) override;
  void WakeUp() override;
  void Add(Dispatcher* dispatcher) override;
  void Remove(Dispatcher* dispatcher) override;
  void Update(Dispatcher* dispatcher) override;

 private:
  // Initialize epoll and eventfd
  bool InitEpoll();

  // Update epoll registration for a dispatcher
  void UpdateEpoll(Dispatcher* dispatcher, bool add);

  // Remove dispatcher from epoll
  void RemoveFromEpoll(Dispatcher* dispatcher);

  // Process pending dispatcher operations
  void ProcessPendingOperations();

  int32_t epoll_fd_;
  int32_t wakeup_fd_;  // eventfd for WakeUp()

  mutable std::mutex mutex_;
  std::set<Dispatcher*> dispatchers_;
  std::vector<Dispatcher*> pending_add_;
  std::vector<Dispatcher*> pending_remove_;
  bool processing_;  // True when processing events
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_PHYSICAL_SOCKET_SERVER_H */
