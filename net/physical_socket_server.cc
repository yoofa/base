/*
 * physical_socket_server.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/physical_socket_server.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <array>
#include <cerrno>
#include <cstring>

#include "base/logging.h"
#include "base/net/socket_dispatcher.h"

namespace ave {
namespace base {
namespace net {

namespace {
constexpr int32_t kMaxEpollEvents = 64;

uint32_t DispatcherEventsToEpollEvents(uint32_t dispatcher_events) {
  uint32_t epoll_events = 0;
  if (dispatcher_events & DE_READ) {
    epoll_events |= EPOLLIN;
  }
  if (dispatcher_events & DE_WRITE) {
    epoll_events |= EPOLLOUT;
  }
  if (dispatcher_events & DE_CONNECT) {
    epoll_events |= EPOLLOUT;
  }
  // Use edge-triggered mode for efficiency
  epoll_events |= EPOLLET;
  return epoll_events;
}

uint32_t EpollEventsToDispatcherEvents(uint32_t epoll_events) {
  uint32_t dispatcher_events = 0;
  if (epoll_events & EPOLLIN) {
    dispatcher_events |= DE_READ;
  }
  if (epoll_events & EPOLLOUT) {
    dispatcher_events |= DE_WRITE | DE_CONNECT;
  }
  if (epoll_events & (EPOLLERR | EPOLLHUP)) {
    dispatcher_events |= DE_CLOSE;
  }
  return dispatcher_events;
}
}  // namespace

PhysicalSocketServer::PhysicalSocketServer()
    : epoll_fd_(-1), wakeup_fd_(-1), processing_(false) {
  if (!InitEpoll()) {
    AVE_LOG(LS_ERROR) << "Failed to initialize epoll";
  }
}

PhysicalSocketServer::~PhysicalSocketServer() {
  if (wakeup_fd_ >= 0) {
    ::close(wakeup_fd_);
  }
  if (epoll_fd_ >= 0) {
    ::close(epoll_fd_);
  }
}

bool PhysicalSocketServer::InitEpoll() {
  epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd_ < 0) {
    AVE_LOG(LS_ERROR) << "epoll_create1 failed: " << strerror(errno);
    return false;
  }

  wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (wakeup_fd_ < 0) {
    AVE_LOG(LS_ERROR) << "eventfd failed: " << strerror(errno);
    ::close(epoll_fd_);
    epoll_fd_ = -1;
    return false;
  }

  // Add wakeup_fd to epoll
  struct epoll_event ev {};
  ev.events = EPOLLIN;
  ev.data.ptr = nullptr;  // nullptr means wakeup event
  if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wakeup_fd_, &ev) < 0) {
    AVE_LOG(LS_ERROR) << "epoll_ctl ADD wakeup_fd failed: " << strerror(errno);
    ::close(wakeup_fd_);
    ::close(epoll_fd_);
    wakeup_fd_ = -1;
    epoll_fd_ = -1;
    return false;
  }

  return true;
}

Socket* PhysicalSocketServer::CreateSocket(int32_t family, int32_t type) {
  return new SocketDispatcher(this, family, type);
}

bool PhysicalSocketServer::Wait(int32_t cms) {
  if (epoll_fd_ < 0) {
    return false;
  }

  // Process any pending add/remove operations
  ProcessPendingOperations();

  std::array<struct epoll_event, kMaxEpollEvents> events{};
  int32_t timeout_ms = cms;

  {
    std::scoped_lock lock(mutex_);
    processing_ = true;
  }

  int32_t nfds =
      ::epoll_wait(epoll_fd_, events.data(), kMaxEpollEvents, timeout_ms);

  {
    std::scoped_lock lock(mutex_);
    processing_ = false;
  }

  if (nfds < 0) {
    if (errno != EINTR) {
      AVE_LOG(LS_ERROR) << "epoll_wait failed: " << strerror(errno);
      return false;
    }
    return true;  // Interrupted, not an error
  }

  if (nfds == 0) {
    return false;  // Timeout
  }

  for (int32_t i = 0; i < nfds; ++i) {
    if (events[i].data.ptr == nullptr) {
      // Wakeup event - drain the eventfd
      uint64_t val{};
      ::read(wakeup_fd_, &val, sizeof(val));
      continue;
    }

    auto* dispatcher = static_cast<Dispatcher*>(events[i].data.ptr);

    // Check if dispatcher is still valid
    {
      std::scoped_lock lock(mutex_);
      if (dispatchers_.find(dispatcher) == dispatchers_.end()) {
        continue;  // Dispatcher was removed
      }
    }

    int32_t error = 0;
    if (events[i].events & EPOLLERR) {
      int32_t sock_error = 0;
      socklen_t len = sizeof(sock_error);
      if (::getsockopt(dispatcher->GetDescriptor(), SOL_SOCKET, SO_ERROR,
                       &sock_error, &len) == 0) {
        error = sock_error;
      }
    }

    uint32_t dispatcher_events =
        EpollEventsToDispatcherEvents(events[i].events);
    dispatcher->OnEvent(dispatcher_events, error);
  }

  // Process any pending operations that occurred during event processing
  ProcessPendingOperations();

  return true;
}

void PhysicalSocketServer::WakeUp() {
  if (wakeup_fd_ >= 0) {
    uint64_t val = 1;
    ::write(wakeup_fd_, &val, sizeof(val));
  }
}

void PhysicalSocketServer::Add(Dispatcher* dispatcher) {
  if (!dispatcher) {
    return;
  }

  std::scoped_lock lock(mutex_);
  if (processing_) {
    pending_add_.push_back(dispatcher);
  } else {
    dispatchers_.insert(dispatcher);
    UpdateEpoll(dispatcher, true);
  }
}

void PhysicalSocketServer::Remove(Dispatcher* dispatcher) {
  if (!dispatcher) {
    return;
  }

  std::scoped_lock lock(mutex_);
  if (processing_) {
    pending_remove_.push_back(dispatcher);
  } else {
    dispatchers_.erase(dispatcher);
    RemoveFromEpoll(dispatcher);
  }
}

void PhysicalSocketServer::Update(Dispatcher* dispatcher) {
  if (!dispatcher) {
    return;
  }

  std::scoped_lock lock(mutex_);
  if (dispatchers_.find(dispatcher) != dispatchers_.end()) {
    UpdateEpoll(dispatcher, false);
  }
}

void PhysicalSocketServer::UpdateEpoll(Dispatcher* dispatcher, bool add) {
  if (epoll_fd_ < 0 || dispatcher->GetDescriptor() < 0) {
    return;
  }

  struct epoll_event ev {};
  ev.events = DispatcherEventsToEpollEvents(dispatcher->GetRequestedEvents());
  ev.data.ptr = dispatcher;

  int32_t op = add ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
  if (::epoll_ctl(epoll_fd_, op, dispatcher->GetDescriptor(), &ev) < 0) {
    if (errno == EEXIST && add) {
      // Already added, try to modify instead
      ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, dispatcher->GetDescriptor(), &ev);
    } else if (errno != ENOENT) {
      AVE_LOG(LS_WARNING) << "epoll_ctl failed: " << strerror(errno);
    }
  }
}

void PhysicalSocketServer::RemoveFromEpoll(Dispatcher* dispatcher) {
  if (epoll_fd_ < 0 || dispatcher->GetDescriptor() < 0) {
    return;
  }

  ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, dispatcher->GetDescriptor(), nullptr);
}

void PhysicalSocketServer::ProcessPendingOperations() {
  std::scoped_lock lock(mutex_);

  // Process pending removals first
  for (auto* dispatcher : pending_remove_) {
    dispatchers_.erase(dispatcher);
    RemoveFromEpoll(dispatcher);
  }
  pending_remove_.clear();

  // Process pending additions
  for (auto* dispatcher : pending_add_) {
    dispatchers_.insert(dispatcher);
    UpdateEpoll(dispatcher, true);
  }
  pending_add_.clear();
}

}  // namespace net
}  // namespace base
}  // namespace ave
