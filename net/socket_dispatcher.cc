/*
 * socket_dispatcher.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/socket_dispatcher.h"

#include <cerrno>

#include "base/net/physical_socket_server.h"

namespace ave {
namespace base {
namespace net {

SocketDispatcher::SocketDispatcher(PhysicalSocketServer* ss,
                                   int32_t family,
                                   int32_t type)
    : PhysicalSocket(ss, family, type), registered_(false) {}

SocketDispatcher::SocketDispatcher(PhysicalSocketServer* ss,
                                   int32_t socket_fd,
                                   int32_t family,
                                   int32_t type)
    : PhysicalSocket(ss, socket_fd, family, type), registered_(false) {}

SocketDispatcher::~SocketDispatcher() {
  RemoveFromServer();
}

int32_t SocketDispatcher::Bind(const SocketAddress& addr) {
  int32_t result = PhysicalSocket::Bind(addr);
  if (result == 0) {
    MaybeAddToServer();
  }
  return result;
}

int32_t SocketDispatcher::Connect(const SocketAddress& addr) {
  int32_t result = PhysicalSocket::Connect(addr);
  if (result == 0 || IsBlocking()) {
    MaybeAddToServer();
  }
  return result;
}

int32_t SocketDispatcher::Listen(int32_t backlog) {
  int32_t result = PhysicalSocket::Listen(backlog);
  if (result == 0) {
    MaybeAddToServer();
  }
  return result;
}

Socket* SocketDispatcher::Accept(SocketAddress* paddr) {
  sockaddr_in saddr{};
  socklen_t addr_len = sizeof(saddr);

  int32_t new_fd =
      ::accept(GetSocketFD(), reinterpret_cast<sockaddr*>(&saddr), &addr_len);
  if (new_fd < 0) {
    SetError(errno);
    return nullptr;
  }

  if (paddr) {
    paddr->FromSockAddr(saddr);
  }

  auto* dispatcher =
      new SocketDispatcher(socket_server_, new_fd, family_, type_);
  dispatcher->MaybeAddToServer();
  return dispatcher;
}

int32_t SocketDispatcher::Close() {
  RemoveFromServer();
  return PhysicalSocket::Close();
}

int32_t SocketDispatcher::GetDescriptor() {
  return GetSocketFD();
}

bool SocketDispatcher::IsDescriptorClosed() {
  return GetSocketFD() < 0;
}

uint32_t SocketDispatcher::GetRequestedEvents() {
  uint32_t events = 0;

  if (GetState() == CS_CONNECTING) {
    // For connecting sockets, we need both read and write events
    // to detect connection completion or failure
    events |= DE_READ | DE_WRITE | DE_CONNECT;
  } else if (GetState() == CS_CONNECTED || type_ == SOCK_DGRAM) {
    // Always interested in read events
    events |= DE_READ;
    // We'll add DE_WRITE when there's pending data to send
    // For now, always request write to detect when ready
    events |= DE_WRITE;
  }

  return events;
}

void SocketDispatcher::OnEvent(uint32_t events, int32_t error) {
  if (error != 0) {
    SetError(error);
    SignalCloseEvent(this, error);
    return;
  }

  // Handle connection completion
  if ((events & DE_CONNECT) && GetState() == CS_CONNECTING) {
    OnConnectComplete();
    if (GetState() == CS_CONNECTED) {
      SignalConnectEvent(this);
    } else {
      SignalCloseEvent(this, GetError());
      return;
    }
  }

  // Handle read events
  if (events & DE_READ) {
    // FIXME(youfa): check if this is a listening socket and handle accept
    // events accordingly
    // NOLINTBEGIN(bugprone-branch-clone)
    if (type_ == SOCK_STREAM && GetState() == CS_CONNECTING) {
      // For TCP listening sockets, this is an accept event
      SignalReadEvent(this);
    } else {
      SignalReadEvent(this);
    }
    // NOLINTEND(bugprone-branch-clone)
  }

  // Handle write events
  if (events & DE_WRITE) {
    SignalWriteEvent(this);
  }

  // Handle close events
  if (events & DE_CLOSE) {
    SignalCloseEvent(this, 0);
  }
}

void SocketDispatcher::MaybeAddToServer() {
  if (!registered_ && socket_server_ && GetSocketFD() >= 0) {
    socket_server_->Add(this);
    registered_ = true;
  }
}

void SocketDispatcher::RemoveFromServer() {
  if (registered_ && socket_server_) {
    socket_server_->Remove(this);
    registered_ = false;
  }
}

}  // namespace net
}  // namespace base
}  // namespace ave
