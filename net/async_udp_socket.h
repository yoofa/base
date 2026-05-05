/*
 * async_udp_socket.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_ASYNC_UDP_SOCKET_H
#define BASE_NET_ASYNC_UDP_SOCKET_H

#include <cstdint>
#include <memory>

#include "base/net/async_packet_socket.h"
#include "base/net/socket.h"

namespace ave {
namespace base {
namespace net {

// AsyncUDPSocket provides asynchronous UDP socket functionality.
// It wraps a Socket and emits signals when data is available.
class AsyncUDPSocket : public AsyncPacketSocket {
 public:
  // Creates an AsyncUDPSocket that takes ownership of the given socket.
  // The socket should already be created and optionally bound.
  explicit AsyncUDPSocket(Socket* socket);
  ~AsyncUDPSocket() override;

  // Disallow copy
  AsyncUDPSocket(const AsyncUDPSocket&) = delete;
  AsyncUDPSocket& operator=(const AsyncUDPSocket&) = delete;

  // Creates and binds an AsyncUDPSocket to the given local address.
  // Returns nullptr on failure.
  static AsyncUDPSocket* Create(Socket* socket, const SocketAddress& bind_addr);

  // AsyncPacketSocket interface
  SocketAddress GetLocalAddress() const override;
  SocketAddress GetRemoteAddress() const override;
  int32_t Send(const void* data, size_t size) override;
  int32_t SendTo(const void* data,
                 size_t size,
                 const SocketAddress& addr) override;
  int32_t Close() override;
  State GetState() const override;
  int32_t GetOption(PacketSocketOption opt, int32_t* value) override;
  int32_t SetOption(PacketSocketOption opt, int32_t value) override;
  int32_t GetError() const override;

 private:
  // Socket signal handlers
  void OnReadEvent(Socket* socket);
  void OnWriteEvent(Socket* socket);

  std::unique_ptr<Socket> socket_;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_ASYNC_UDP_SOCKET_H */
