/*
 * socket_thread.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/socket_thread.h"

#include <chrono>

#include "base/logging.h"
#include "base/net/physical_socket_server.h"

namespace ave {
namespace base {
namespace net {

namespace {
// Thread-local pointer to current SocketThread
thread_local SocketThread* g_current_thread = nullptr;
}  // namespace

SocketThread* CurrentSocketThread() {
  return g_current_thread;
}

SocketThread::SocketThread()
    : SocketThread(std::make_unique<PhysicalSocketServer>()) {}

SocketThread::SocketThread(std::unique_ptr<SocketServer> socket_server)
    : socket_server_(std::move(socket_server)),
      running_(false),
      owned_thread_(false) {}

SocketThread::~SocketThread() {
  Stop();
}

void SocketThread::Start() {
  if (running_) {
    return;
  }
  running_ = true;
  owned_thread_ = true;
  thread_ = std::thread(&SocketThread::RunInternal, this);
}

void SocketThread::Stop() {
  if (!running_) {
    return;
  }
  running_ = false;
  socket_server_->WakeUp();

  if (owned_thread_ && thread_.joinable()) {
    thread_.join();
  }
}

bool SocketThread::IsCurrent() const {
  return std::this_thread::get_id() == thread_id_;
}

void SocketThread::PostTask(std::function<void()> task) {
  {
    std::scoped_lock lock(task_mutex_);
    tasks_.push(std::move(task));
  }
  socket_server_->WakeUp();
}

void SocketThread::PostDelayedTask(std::function<void()> task,
                                   int64_t delay_ms) {
  DelayedTask delayed;
  delayed.task = std::move(task);
  delayed.run_time_ms = CurrentTimeMs() + delay_ms;

  {
    std::scoped_lock lock(task_mutex_);
    delayed_tasks_.push(std::move(delayed));
  }
  socket_server_->WakeUp();
}

void SocketThread::Invoke(std::function<void()> task) {
  if (IsCurrent()) {
    // Already on this thread, just run it
    task();
    return;
  }

  // Post task and wait for completion
  std::atomic<bool> done{false};

  PostTask([&]() {
    task();
    {
      std::scoped_lock lock(invoke_mutex_);
      done = true;
    }
    invoke_cv_.notify_one();
  });

  // Wait for task to complete
  std::unique_lock<std::mutex> lock(invoke_mutex_);
  invoke_cv_.wait(lock, [&]() { return done.load(); });
}

Socket* SocketThread::CreateSocket(int32_t family, int32_t type) {
  return socket_server_->CreateSocket(family, type);
}

void SocketThread::Run() {
  thread_id_ = std::this_thread::get_id();
  g_current_thread = this;
  running_ = true;

  AVE_LOG(LS_INFO) << "SocketThread running";

  while (running_) {
    ProcessMessages(100);
  }

  g_current_thread = nullptr;
  AVE_LOG(LS_INFO) << "SocketThread stopped";
}

bool SocketThread::ProcessMessages(int32_t wait_ms) {
  if (!running_) {
    return false;
  }

  // Process pending tasks first
  ProcessTasks();

  // Calculate actual wait time based on delayed tasks
  int64_t next_delay = GetNextDelayMs();
  int32_t actual_wait = wait_ms;
  if (next_delay >= 0 && next_delay < wait_ms) {
    actual_wait = static_cast<int32_t>(next_delay);
  }

  // Wait for network events or wakeup
  socket_server_->Wait(actual_wait);

  // Process tasks again (might have been woken up by new task)
  ProcessTasks();

  return running_;
}

std::unique_ptr<SocketThread> SocketThread::WrapCurrent() {
  auto thread = std::make_unique<SocketThread>();
  thread->thread_id_ = std::this_thread::get_id();
  thread->owned_thread_ = false;
  thread->running_ = true;
  g_current_thread = thread.get();
  return thread;
}

void SocketThread::RunInternal() {
  thread_id_ = std::this_thread::get_id();
  g_current_thread = this;

  AVE_LOG(LS_INFO) << "SocketThread started";

  while (running_) {
    ProcessMessages(100);
  }

  g_current_thread = nullptr;
  AVE_LOG(LS_INFO) << "SocketThread stopped";
}

void SocketThread::ProcessTasks() {
  // Process immediate tasks
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

  // Process delayed tasks that are ready
  int64_t now = CurrentTimeMs();
  std::vector<std::function<void()>> ready_tasks;

  {
    std::scoped_lock lock(task_mutex_);
    while (!delayed_tasks_.empty() && delayed_tasks_.top().run_time_ms <= now) {
      // TODO(youfa): remove const_cast by changing the priority_queue to hold
      // non-const DelayedTask
      // NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
      ready_tasks.push_back(
          std::move(const_cast<DelayedTask&>(delayed_tasks_.top()).task));
      // NOLINTEND(cppcoreguidelines-pro-type-const-cast)
      delayed_tasks_.pop();
    }
  }

  for (auto& task : ready_tasks) {
    task();
  }
}

int64_t SocketThread::GetNextDelayMs() {
  std::scoped_lock lock(task_mutex_);
  if (delayed_tasks_.empty()) {
    return -1;
  }
  int64_t delay = delayed_tasks_.top().run_time_ms - CurrentTimeMs();
  return delay > 0 ? delay : 0;
}

int64_t SocketThread::CurrentTimeMs() {
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             now.time_since_epoch())
      .count();
}

}  // namespace net
}  // namespace base
}  // namespace ave
