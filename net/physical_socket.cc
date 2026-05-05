/*
 * physical_socket.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/physical_socket.h"

#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>

#include "base/logging.h"

namespace ave {
namespace base {
namespace net {

PhysicalSocket::PhysicalSocket(PhysicalSocketServer* ss,
                               int32_t family,
                               int32_t type)
    : socket_server_(ss),
      socket_fd_(-1),
      family_(family),
      type_(type),
      error_(0),
      state_(CS_CLOSED) {
  if (CreateSocket(family, type) != 0) {
    AVE_LOG(LS_ERROR) << "Failed to create socket";
  }
}

PhysicalSocket::PhysicalSocket(PhysicalSocketServer* ss,
                               int32_t socket_fd,
                               int32_t family,
                               int32_t type)
    : socket_server_(ss),
      socket_fd_(socket_fd),
      family_(family),
      type_(type),
      error_(0),
      state_(CS_CONNECTED) {
  if (socket_fd_ >= 0) {
    SetNonBlocking(socket_fd_);
  }
}

PhysicalSocket::~PhysicalSocket() {
  Close();
}

int32_t PhysicalSocket::CreateSocket(int32_t family, int32_t type) {
  socket_fd_ = ::socket(family, type, 0);
  if (socket_fd_ < 0) {
    error_ = errno;
    return -1;
  }
  if (SetNonBlocking(socket_fd_) != 0) {
    ::close(socket_fd_);
    socket_fd_ = -1;
    return -1;
  }
  return 0;
}

int32_t PhysicalSocket::SetNonBlocking(int32_t fd) {
  int32_t flags = ::fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    error_ = errno;
    return -1;
  }
  if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    error_ = errno;
    return -1;
  }
  return 0;
}

SocketAddress PhysicalSocket::GetLocalAddress() const {
  sockaddr_storage addr_storage{};
  socklen_t addr_len = sizeof(addr_storage);
  auto* addr = reinterpret_cast<sockaddr*>(&addr_storage);

  if (socket_fd_ < 0 || ::getsockname(socket_fd_, addr, &addr_len) < 0) {
    return {};
  }

  SocketAddress result;
  if (addr->sa_family == AF_INET) {
    result.FromSockAddr(*reinterpret_cast<sockaddr_in*>(addr));
  }
  return result;
}

SocketAddress PhysicalSocket::GetRemoteAddress() const {
  sockaddr_storage addr_storage{};
  socklen_t addr_len = sizeof(addr_storage);
  auto* addr = reinterpret_cast<sockaddr*>(&addr_storage);

  if (socket_fd_ < 0 || ::getpeername(socket_fd_, addr, &addr_len) < 0) {
    return {};
  }

  SocketAddress result;
  if (addr->sa_family == AF_INET) {
    result.FromSockAddr(*reinterpret_cast<sockaddr_in*>(addr));
  }
  return result;
}

int32_t PhysicalSocket::Bind(const SocketAddress& addr) {
  sockaddr_in saddr{};
  addr.ToSockAddr(&saddr);

  int32_t err =
      ::bind(socket_fd_, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr));
  if (err < 0) {
    error_ = errno;
    return -1;
  }
  local_addr_ = addr;
  return 0;
}

int32_t PhysicalSocket::Connect(const SocketAddress& addr) {
  sockaddr_in saddr{};
  addr.ToSockAddr(&saddr);

  int32_t err =
      ::connect(socket_fd_, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr));
  if (err == 0) {
    state_ = CS_CONNECTED;
    remote_addr_ = addr;
    return 0;
  }

  if (errno == EINPROGRESS || errno == EWOULDBLOCK) {
    state_ = CS_CONNECTING;
    remote_addr_ = addr;
    return 0;
  }

  error_ = errno;
  return -1;
}

int32_t PhysicalSocket::Send(const void* pv, size_t cb) {
  ssize_t sent = ::send(socket_fd_, pv, cb, MSG_NOSIGNAL);
  if (sent < 0) {
    error_ = errno;
    return -1;
  }
  return static_cast<int32_t>(sent);
}

int32_t PhysicalSocket::SendTo(const void* pv,
                               size_t cb,
                               const SocketAddress& addr) {
  sockaddr_in saddr{};
  addr.ToSockAddr(&saddr);

  ssize_t sent = ::sendto(socket_fd_, pv, cb, MSG_NOSIGNAL,
                          reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr));
  if (sent < 0) {
    error_ = errno;
    return -1;
  }
  return static_cast<int32_t>(sent);
}

int32_t PhysicalSocket::Recv(void* pv, size_t cb, int64_t* timestamp) {
  if (timestamp) {
    *timestamp = -1;
  }
  ssize_t received = ::recv(socket_fd_, pv, cb, 0);
  if (received < 0) {
    error_ = errno;
    return -1;
  }
  return static_cast<int32_t>(received);
}

int32_t PhysicalSocket::RecvFrom(void* pv,
                                 size_t cb,
                                 SocketAddress* paddr,
                                 int64_t* timestamp) {
  if (timestamp) {
    *timestamp = -1;
  }

  sockaddr_in saddr{};
  socklen_t addr_len = sizeof(saddr);

  ssize_t received = ::recvfrom(socket_fd_, pv, cb, 0,
                                reinterpret_cast<sockaddr*>(&saddr), &addr_len);
  if (received < 0) {
    error_ = errno;
    return -1;
  }

  if (paddr) {
    paddr->FromSockAddr(saddr);
  }
  return static_cast<int32_t>(received);
}

int32_t PhysicalSocket::Listen(int32_t backlog) {
  int32_t err = ::listen(socket_fd_, backlog);
  if (err < 0) {
    error_ = errno;
    return -1;
  }
  state_ = CS_CONNECTING;  // Listening state
  return 0;
}

Socket* PhysicalSocket::Accept(SocketAddress* paddr) {
  sockaddr_in saddr{};
  socklen_t addr_len = sizeof(saddr);

  int32_t new_fd =
      ::accept(socket_fd_, reinterpret_cast<sockaddr*>(&saddr), &addr_len);
  if (new_fd < 0) {
    error_ = errno;
    return nullptr;
  }

  if (paddr) {
    paddr->FromSockAddr(saddr);
  }

  return new PhysicalSocket(socket_server_, new_fd, family_, type_);
}

int32_t PhysicalSocket::Close() {
  if (socket_fd_ >= 0) {
    ::close(socket_fd_);
    socket_fd_ = -1;
  }
  state_ = CS_CLOSED;
  return 0;
}

int32_t PhysicalSocket::GetError() const {
  return error_;
}

void PhysicalSocket::SetError(int32_t error) {
  error_ = error;
}

Socket::ConnState PhysicalSocket::GetState() const {
  return state_;
}

int32_t PhysicalSocket::GetOption(Option opt, int32_t* value) {
  int32_t level = SOL_SOCKET;
  int32_t optname = 0;

  switch (opt) {
    case OPT_RCVBUF:
      optname = SO_RCVBUF;
      break;
    case OPT_SNDBUF:
      optname = SO_SNDBUF;
      break;
    case OPT_NODELAY:
      level = IPPROTO_TCP;
      optname = TCP_NODELAY;
      break;
    case OPT_IPV6_V6ONLY:
      level = IPPROTO_IPV6;
      optname = IPV6_V6ONLY;
      break;
    default:
      return -1;
  }

  socklen_t optlen = sizeof(*value);
  int32_t err = ::getsockopt(socket_fd_, level, optname, value, &optlen);
  if (err < 0) {
    error_ = errno;
    return -1;
  }
  return 0;
}

int32_t PhysicalSocket::SetOption(Option opt, int32_t value) {
  int32_t level = SOL_SOCKET;
  int32_t optname = 0;

  switch (opt) {
    case OPT_RCVBUF:
      optname = SO_RCVBUF;
      break;
    case OPT_SNDBUF:
      optname = SO_SNDBUF;
      break;
    case OPT_NODELAY:
      level = IPPROTO_TCP;
      optname = TCP_NODELAY;
      break;
    case OPT_IPV6_V6ONLY:
      level = IPPROTO_IPV6;
      optname = IPV6_V6ONLY;
      break;
    default:
      return -1;
  }

  int32_t err = ::setsockopt(socket_fd_, level, optname, &value, sizeof(value));
  if (err < 0) {
    error_ = errno;
    return -1;
  }
  return 0;
}

void PhysicalSocket::OnConnectComplete() {
  if (state_ == CS_CONNECTING) {
    int32_t err = 0;
    socklen_t len = sizeof(err);
    if (::getsockopt(socket_fd_, SOL_SOCKET, SO_ERROR, &err, &len) == 0 &&
        err == 0) {
      state_ = CS_CONNECTED;
    } else {
      error_ = err;
      state_ = CS_CLOSED;
    }
  }
}

}  // namespace net
}  // namespace base
}  // namespace ave
