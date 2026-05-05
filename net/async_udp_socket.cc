/*
 * async_udp_socket.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/async_udp_socket.h"

#include <array>
#include <cstring>

#include "base/logging.h"

namespace ave {
namespace base {
namespace net {

namespace {
constexpr size_t kMaxUDPPacketSize = 65535;
}  // namespace

AsyncUDPSocket::AsyncUDPSocket(Socket* socket) : socket_(socket) {
  if (socket_) {
    socket_->SignalReadEvent.connect(this, &AsyncUDPSocket::OnReadEvent);
    socket_->SignalWriteEvent.connect(this, &AsyncUDPSocket::OnWriteEvent);
  }
}

AsyncUDPSocket::~AsyncUDPSocket() {
  if (socket_) {
    socket_->SignalReadEvent.disconnect(this);
    socket_->SignalWriteEvent.disconnect(this);
  }
}

AsyncUDPSocket* AsyncUDPSocket::Create(Socket* socket,
                                       const SocketAddress& bind_addr) {
  if (!socket) {
    return nullptr;
  }

  if (socket->Bind(bind_addr) < 0) {
    AVE_LOG(LS_ERROR) << "Failed to bind UDP socket to "
                      << bind_addr.ToString();
    delete socket;
    return nullptr;
  }

  return new AsyncUDPSocket(socket);
}

SocketAddress AsyncUDPSocket::GetLocalAddress() const {
  if (socket_) {
    return socket_->GetLocalAddress();
  }
  return {};
}

SocketAddress AsyncUDPSocket::GetRemoteAddress() const {
  if (socket_) {
    return socket_->GetRemoteAddress();
  }
  return {};
}

int32_t AsyncUDPSocket::Send(const void* data, size_t size) {
  if (!socket_) {
    return -1;
  }
  return socket_->Send(data, size);
}

int32_t AsyncUDPSocket::SendTo(const void* data,
                               size_t size,
                               const SocketAddress& addr) {
  if (!socket_) {
    return -1;
  }
  return socket_->SendTo(data, size, addr);
}

int32_t AsyncUDPSocket::Close() {
  if (socket_) {
    return socket_->Close();
  }
  return 0;
}

AsyncPacketSocket::State AsyncUDPSocket::GetState() const {
  if (!socket_) {
    return STATE_CLOSED;
  }
  // UDP sockets are considered bound after successful bind
  return STATE_BOUND;
}

int32_t AsyncUDPSocket::GetOption(PacketSocketOption opt, int32_t* value) {
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

int32_t AsyncUDPSocket::SetOption(PacketSocketOption opt, int32_t value) {
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

int32_t AsyncUDPSocket::GetError() const {
  if (socket_) {
    return socket_->GetError();
  }
  return 0;
}

void AsyncUDPSocket::OnReadEvent(Socket* socket [[maybe_unused]]) {
  std::array<uint8_t, kMaxUDPPacketSize> buf{};
  SocketAddress remote_addr;
  int64_t timestamp = -1;

  int32_t len =
      socket_->RecvFrom(buf.data(), buf.size(), &remote_addr, &timestamp);
  if (len > 0) {
    SignalReadPacket(this, buf.data(), static_cast<size_t>(len), remote_addr,
                     timestamp);
  } else if (len < 0 && !socket_->IsBlocking()) {
    SignalClose(this, socket_->GetError());
  }
}

void AsyncUDPSocket::OnWriteEvent(Socket* socket [[maybe_unused]]) {
  SignalReadyToSend(this);
}

}  // namespace net
}  // namespace base
}  // namespace ave
