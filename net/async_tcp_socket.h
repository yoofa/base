/*
 * async_tcp_socket.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_ASYNC_TCP_SOCKET_H
#define BASE_NET_ASYNC_TCP_SOCKET_H

#include <cstdint>
#include <memory>

#include "base/buffer.h"
#include "base/net/async_packet_socket.h"
#include "base/net/socket.h"

namespace ave {
namespace base {
namespace net {

// AsyncTCPSocket provides asynchronous TCP socket functionality.
// It manages connection state and provides packet-based I/O over TCP stream.
class AsyncTCPSocket : public AsyncPacketSocket {
 public:
  // Creates an AsyncTCPSocket that takes ownership of the given socket.
  explicit AsyncTCPSocket(Socket* socket);
  ~AsyncTCPSocket() override;

  // Disallow copy
  AsyncTCPSocket(const AsyncTCPSocket&) = delete;
  AsyncTCPSocket& operator=(const AsyncTCPSocket&) = delete;

  // Creates a client AsyncTCPSocket and initiates connection.
  // Returns nullptr on failure.
  static AsyncTCPSocket* Create(Socket* socket,
                                const SocketAddress& bind_addr,
                                const SocketAddress& remote_addr);

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
  void OnConnectEvent(Socket* socket);
  void OnCloseEvent(Socket* socket, int32_t error);

  // Flush pending data in the write buffer
  void FlushWriteBuffer();

  std::unique_ptr<Socket> socket_;
  base::Buffer read_buffer_;
  base::Buffer write_buffer_;
  bool connected_;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_ASYNC_TCP_SOCKET_H */
