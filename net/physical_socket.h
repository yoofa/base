/*
 * physical_socket.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_PHYSICAL_SOCKET_H
#define BASE_NET_PHYSICAL_SOCKET_H

#include <cstdint>

#include "base/net/socket.h"
#include "base/net/socket_address.h"

namespace ave {
namespace base {
namespace net {

class PhysicalSocketServer;

// PhysicalSocket wraps a native system socket with non-blocking I/O.
class PhysicalSocket : public Socket {
 public:
  PhysicalSocket(PhysicalSocketServer* ss, int32_t family, int32_t type);
  ~PhysicalSocket() override;

  // Disallow copy
  PhysicalSocket(const PhysicalSocket&) = delete;
  PhysicalSocket& operator=(const PhysicalSocket&) = delete;

  // Socket interface
  SocketAddress GetLocalAddress() const override;
  SocketAddress GetRemoteAddress() const override;
  int32_t Bind(const SocketAddress& addr) override;
  int32_t Connect(const SocketAddress& addr) override;
  int32_t Send(const void* pv, size_t cb) override;
  int32_t SendTo(const void* pv, size_t cb, const SocketAddress& addr) override;
  int32_t Recv(void* pv, size_t cb, int64_t* timestamp) override;
  int32_t RecvFrom(void* pv,
                   size_t cb,
                   SocketAddress* paddr,
                   int64_t* timestamp) override;
  int32_t Listen(int32_t backlog) override;
  Socket* Accept(SocketAddress* paddr) override;
  int32_t Close() override;
  int32_t GetError() const override;
  void SetError(int32_t error) override;
  ConnState GetState() const override;
  int32_t GetOption(Option opt, int32_t* value) override;
  int32_t SetOption(Option opt, int32_t value) override;

  // Returns the underlying socket file descriptor
  int32_t GetSocketFD() const { return socket_fd_; }

 protected:
  // For subclasses to create socket with an existing fd
  PhysicalSocket(PhysicalSocketServer* ss,
                 int32_t socket_fd,
                 int32_t family,
                 int32_t type);

  // Creates the actual socket
  int32_t CreateSocket(int32_t family, int32_t type);

  // Sets socket to non-blocking mode
  int32_t SetNonBlocking(int32_t fd);

  // Updates the connection state
  void SetState(ConnState state) { state_ = state; }

  // Called when connection completes
  void OnConnectComplete();

  PhysicalSocketServer* socket_server_;
  int32_t socket_fd_;
  int32_t family_;
  int32_t type_;
  int32_t error_;
  ConnState state_;
  SocketAddress local_addr_;
  SocketAddress remote_addr_;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_PHYSICAL_SOCKET_H */
