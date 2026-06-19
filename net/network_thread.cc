/*
 * network_thread.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/network_thread.h"

#include "base/logging.h"

namespace ave {
namespace base {
namespace net {

NetworkThread::NetworkThread() : running_(false) {
  socket_server_ = std::make_unique<PhysicalSocketServer>();
}

NetworkThread::~NetworkThread() {
  Stop();
}

void NetworkThread::Start() {
  if (running_) {
    return;
  }
  running_ = true;
  thread_ = std::thread(&NetworkThread::Run, this);
}

void NetworkThread::Stop() {
  if (!running_) {
    return;
  }
  running_ = false;
  socket_server_->WakeUp();
  if (thread_.joinable()) {
    thread_.join();
  }
}

bool NetworkThread::IsCurrent() const {
  return std::this_thread::get_id() == thread_id_;
}

void NetworkThread::PostTask(std::function<void()> task) {
  {
    std::scoped_lock lock(task_mutex_);
    tasks_.push(std::move(task));
  }
  socket_server_->WakeUp();
}

Socket* NetworkThread::CreateSocket(int32_t family, int32_t type) {
  return socket_server_->CreateSocket(family, type);
}

void NetworkThread::Run() {
  thread_id_ = std::this_thread::get_id();
  AVE_LOG(LS_INFO) << "NetworkThread started";

  while (running_) {
    // Process pending tasks
    ProcessTasks();

    // Wait for network events (100ms timeout)
    socket_server_->Wait(100);
  }

  // Process any remaining tasks before exit
  ProcessTasks();

  AVE_LOG(LS_INFO) << "NetworkThread stopped";
}

void NetworkThread::ProcessTasks() {
  std::queue<std::function<void()>> tasks_to_run;
  {
    std::scoped_lock lock(task_mutex_);
    tasks_to_run.swap(tasks_);
  }

  while (!tasks_to_run.empty()) {
    auto& task = tasks_to_run.front();
    task();
    tasks_to_run.pop();
  }
}

}  // namespace net
}  // namespace base
}  // namespace ave
