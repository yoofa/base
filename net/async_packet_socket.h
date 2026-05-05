/*
 * async_packet_socket.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_ASYNC_PACKET_SOCKET_H
#define BASE_NET_ASYNC_PACKET_SOCKET_H

#include <cstddef>
#include <cstdint>

#include "base/net/socket_address.h"
#include "base/third_party/sigslot/sigslot.h"

namespace ave {
namespace base {
namespace net {

// Options that can be set on an AsyncPacketSocket.
enum class PacketSocketOption {
  kDontFragment,
  kRecvBuf,
  kSendBuf,
};

// AsyncPacketSocket is the user-facing interface for asynchronous
// packet-based network I/O. It uses signals to notify callers of events.
class AsyncPacketSocket : public sigslot::has_slots<> {
 public:
  enum State {
    STATE_CLOSED,
    STATE_BINDING,
    STATE_BOUND,
    STATE_CONNECTING,
    STATE_CONNECTED,
  };

  ~AsyncPacketSocket() override = default;

  // Returns the local address of the socket.
  virtual SocketAddress GetLocalAddress() const = 0;

  // Returns the remote address of the socket (for connected sockets).
  virtual SocketAddress GetRemoteAddress() const = 0;

  // Send a packet to the specified address.
  // Returns the number of bytes sent, or -1 on error.
  virtual int32_t Send(const void* data, size_t size) = 0;

  // Send a packet to the specified address.
  // Returns the number of bytes sent, or -1 on error.
  virtual int32_t SendTo(const void* data,
                         size_t size,
                         const SocketAddress& addr) = 0;

  // Close the socket.
  virtual int32_t Close() = 0;

  // Returns the current state of the socket.
  virtual State GetState() const = 0;

  // Get/Set socket options.
  virtual int32_t GetOption(PacketSocketOption opt, int32_t* value) = 0;
  virtual int32_t SetOption(PacketSocketOption opt, int32_t value) = 0;

  // Returns the last error code.
  virtual int32_t GetError() const = 0;

  // Signal emitted when a packet is received.
  // Parameters: socket, data, size, remote_addr, timestamp (microseconds)
  sigslot::signal5<AsyncPacketSocket*,
                   const uint8_t*,
                   size_t,
                   const SocketAddress&,
                   int64_t>
      SignalReadPacket;

  // Signal emitted when the socket is ready to send data.
  sigslot::signal1<AsyncPacketSocket*> SignalReadyToSend;

  // Signal emitted when the socket is connected (for TCP).
  sigslot::signal1<AsyncPacketSocket*> SignalConnect;

  // Signal emitted when the socket is closed.
  // Parameters: socket, error_code
  sigslot::signal2<AsyncPacketSocket*, int32_t> SignalClose;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_ASYNC_PACKET_SOCKET_H */
