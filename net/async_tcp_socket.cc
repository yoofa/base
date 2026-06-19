/*
 * async_tcp_socket.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/async_tcp_socket.h"
#include <array>

#include <cstring>
#include <utility>

#include "base/logging.h"

namespace ave {
namespace base {
namespace net {

namespace {
constexpr size_t kMaxTCPReadSize = 64 * 1024;
}  // namespace

AsyncTCPSocket::AsyncTCPSocket(Socket* socket)
    : socket_(socket), connected_(false) {
  if (socket_) {
    socket_->SignalReadEvent.connect(this, &AsyncTCPSocket::OnReadEvent);
    socket_->SignalWriteEvent.connect(this, &AsyncTCPSocket::OnWriteEvent);
    socket_->SignalConnectEvent.connect(this, &AsyncTCPSocket::OnConnectEvent);
    socket_->SignalCloseEvent.connect(this, &AsyncTCPSocket::OnCloseEvent);

    if (socket_->GetState() == Socket::CS_CONNECTED) {
      connected_ = true;
    }
  }
}

AsyncTCPSocket::~AsyncTCPSocket() {
  if (socket_) {
    socket_->SignalReadEvent.disconnect(this);
    socket_->SignalWriteEvent.disconnect(this);
    socket_->SignalConnectEvent.disconnect(this);
    socket_->SignalCloseEvent.disconnect(this);
  }
}

AsyncTCPSocket* AsyncTCPSocket::Create(Socket* socket,
                                       const SocketAddress& bind_addr,
                                       const SocketAddress& remote_addr) {
  if (!socket) {
    return nullptr;
  }

  // Bind if local address is specified
  if (!bind_addr.IsNil()) {
    if (socket->Bind(bind_addr) < 0) {
      AVE_LOG(LS_ERROR) << "Failed to bind TCP socket to "
                        << bind_addr.ToString();
      delete socket;
      return nullptr;
    }
  }

  // Connect to remote address
  if (socket->Connect(remote_addr) < 0 && !socket->IsBlocking()) {
    AVE_LOG(LS_ERROR) << "Failed to connect TCP socket to "
                      << remote_addr.ToString();
    delete socket;
    return nullptr;
  }

  return new AsyncTCPSocket(socket);
}

SocketAddress AsyncTCPSocket::GetLocalAddress() const {
  if (socket_) {
    return socket_->GetLocalAddress();
  }
  return {};
}

SocketAddress AsyncTCPSocket::GetRemoteAddress() const {
  if (socket_) {
    return socket_->GetRemoteAddress();
  }
  return {};
}

int32_t AsyncTCPSocket::Send(const void* data, size_t size) {
  if (!socket_) {
    return -1;
  }

  if (!connected_) {
    // Buffer data until connected
    write_buffer_.AppendData(static_cast<const uint8_t*>(data), size);
    return static_cast<int32_t>(size);
  }

  // If we have buffered data, buffer this too and try to flush
  if (!write_buffer_.empty()) {
    write_buffer_.AppendData(static_cast<const uint8_t*>(data), size);
    FlushWriteBuffer();
    return static_cast<int32_t>(size);
  }

  // Try to send directly
  int32_t sent = socket_->Send(data, size);
  if (sent < 0) {
    if (socket_->IsBlocking()) {
      // Buffer the data and try again later
      write_buffer_.AppendData(static_cast<const uint8_t*>(data), size);
      return static_cast<int32_t>(size);
    }
    return -1;
  }

  // If not all data was sent, buffer the rest
  if (std::cmp_less(sent, size)) {
    write_buffer_.AppendData(static_cast<const uint8_t*>(data) + sent,
                             size - sent);
  }

  return static_cast<int32_t>(size);
}

int32_t AsyncTCPSocket::SendTo(const void* data,
                               size_t size,
                               const SocketAddress& addr) {
  // TCP is connection-oriented, SendTo is the same as Send
  (void)addr;
  return Send(data, size);
}

int32_t AsyncTCPSocket::Close() {
  write_buffer_.Clear();
  read_buffer_.Clear();
  connected_ = false;
  if (socket_) {
    return socket_->Close();
  }
  return 0;
}

AsyncPacketSocket::State AsyncTCPSocket::GetState() const {
  if (!socket_) {
    return STATE_CLOSED;
  }

  switch (socket_->GetState()) {
    case Socket::CS_CLOSED:
      return STATE_CLOSED;
    case Socket::CS_CONNECTING:
      return STATE_CONNECTING;
    case Socket::CS_CONNECTED:
      return STATE_CONNECTED;
    default:
      return STATE_CLOSED;
  }
}

int32_t AsyncTCPSocket::GetOption(PacketSocketOption opt, int32_t* value) {
  if (!socket_) {
    return -1;
  }

  Socket::Option sock_opt{};
  switch (opt) {
    case PacketSocketOption::kRecvBuf:
      sock_opt = Socket::OPT_RCVBUF;
      break;
    case PacketSocketOption::kSendBuf:
      sock_opt = Socket::OPT_SNDBUF;
      break;
    case PacketSocketOption::kDontFragment:
      sock_opt = Socket::OPT_DONTFRAGMENT;
      break;
    default:
      return -1;
  }
  return socket_->GetOption(sock_opt, value);
}

int32_t AsyncTCPSocket::SetOption(PacketSocketOption opt, int32_t value) {
  if (!socket_) {
    return -1;
  }

  Socket::Option sock_opt{};
  switch (opt) {
    case PacketSocketOption::kRecvBuf:
      sock_opt = Socket::OPT_RCVBUF;
      break;
    case PacketSocketOption::kSendBuf:
      sock_opt = Socket::OPT_SNDBUF;
      break;
    case PacketSocketOption::kDontFragment:
      sock_opt = Socket::OPT_DONTFRAGMENT;
      break;
    default:
      return -1;
  }
  return socket_->SetOption(sock_opt, value);
}

int32_t AsyncTCPSocket::GetError() const {
  if (socket_) {
    return socket_->GetError();
  }
  return 0;
}

void AsyncTCPSocket::OnReadEvent(Socket* socket [[maybe_unused]]) {
  std::array<uint8_t, kMaxTCPReadSize> buf{};
  int64_t timestamp = -1;

  int32_t len = socket_->Recv(buf.data(), buf.size(), &timestamp);
  if (len > 0) {
    // For TCP, we receive raw bytes. The caller can interpret as packets.
    SignalReadPacket(this, buf.data(), static_cast<size_t>(len),
                     socket_->GetRemoteAddress(), timestamp);
  } else if (len == 0) {
    // Connection closed by peer
    SignalClose(this, 0);
  } else if (!socket_->IsBlocking()) {
    SignalClose(this, socket_->GetError());
  }
}

void AsyncTCPSocket::OnWriteEvent(Socket* socket [[maybe_unused]]) {
  if (!write_buffer_.empty()) {
    FlushWriteBuffer();
  }
  if (write_buffer_.empty()) {
    SignalReadyToSend(this);
  }
}

void AsyncTCPSocket::OnConnectEvent(Socket* socket [[maybe_unused]]) {
  connected_ = true;
  SignalConnect(this);

  // Flush any buffered data
  if (!write_buffer_.empty()) {
    FlushWriteBuffer();
  }
}

void AsyncTCPSocket::OnCloseEvent(Socket* socket [[maybe_unused]],
                                  int32_t error) {
  connected_ = false;
  SignalClose(this, error);
}

void AsyncTCPSocket::FlushWriteBuffer() {
  if (write_buffer_.empty() || !socket_) {
    return;
  }

  int32_t sent = socket_->Send(write_buffer_.data(), write_buffer_.size());
  if (sent > 0) {
    // Remove sent data from buffer
    size_t remaining = write_buffer_.size() - sent;
    if (remaining > 0) {
      std::memmove(write_buffer_.data(), write_buffer_.data() + sent,
                   remaining);
    }
    write_buffer_.SetSize(remaining);
  }
}

}  // namespace net
}  // namespace base
}  // namespace ave
