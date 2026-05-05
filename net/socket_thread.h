/*
 * socket_thread.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * SocketThread implements WebRTC-style thread that combines
 * message queue with SocketServer for unified event handling.
 *
 * Key design from WebRTC:
 * - Thread owns a SocketServer
 * - Wait() handles both network events AND posted tasks
 * - WakeUp() interrupts Wait() when new tasks arrive
 * - Single event loop for everything
 */

#ifndef BASE_NET_SOCKET_THREAD_H
#define BASE_NET_SOCKET_THREAD_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "socket_server.h"

namespace ave {
namespace base {
namespace net {

// SocketThread combines a message queue with SocketServer.
// Similar to rtc::Thread in WebRTC.
//
// Usage:
//   SocketThread thread;
//   thread.Start();
//
//   // Post tasks to run on the thread
//   thread.PostTask([]() { ... });
//
//   // Create sockets on the thread
//   thread.PostTask([&]() {
//       Socket* socket = thread.socket_server()->CreateSocket(...);
//   });
//
//   thread.Stop();
//
class SocketThread {
 public:
  SocketThread();
  explicit SocketThread(std::unique_ptr<SocketServer> socket_server);
  ~SocketThread();

  // Disallow copy
  SocketThread(const SocketThread&) = delete;
  SocketThread& operator=(const SocketThread&) = delete;

  // Start the thread
  void Start();

  // Stop the thread and wait for it to exit
  void Stop();

  // Returns true if called from this thread
  bool IsCurrent() const;

  // Post a task to be executed on this thread
  void PostTask(std::function<void()> task);

  // Post a delayed task (approximate, depends on Wait timeout)
  void PostDelayedTask(std::function<void()> task, int64_t delay_ms);

  // Execute a task synchronously on this thread (blocks caller)
  // WARNING: Do not call from the same thread (deadlock!)
  void Invoke(std::function<void()> task);

  // Get the socket server (only use from this thread!)
  SocketServer* socket_server() { return socket_server_.get(); }

  // Convenience: Create socket (can call from any thread, but
  // the socket should only be used on this thread)
  Socket* CreateSocket(int32_t family, int32_t type);

  // Run the event loop (call from the thread itself)
  // This is called automatically by Start(), but you can also
  // create a SocketThread for the current thread and call Run()
  void Run();

  // Process one iteration of the event loop
  // Returns false if Stop() was called
  bool ProcessMessages(int32_t wait_ms);

  // Wrap current thread (makes current thread a SocketThread)
  static std::unique_ptr<SocketThread> WrapCurrent();

 private:
  struct DelayedTask {
    std::function<void()> task;
    int64_t run_time_ms;

    bool operator>(const DelayedTask& other) const {
      return run_time_ms > other.run_time_ms;
    }
  };

  void RunInternal();
  void ProcessTasks();
  int64_t GetNextDelayMs();

  static int64_t CurrentTimeMs();

  std::unique_ptr<SocketServer> socket_server_;
  std::thread thread_;
  std::thread::id thread_id_;
  std::atomic<bool> running_;
  bool owned_thread_;  // true if we created the thread

  std::mutex task_mutex_;
  std::queue<std::function<void()>> tasks_;
  std::priority_queue<DelayedTask, std::vector<DelayedTask>, std::greater<>>
      delayed_tasks_;

  // For Invoke() synchronization
  std::mutex invoke_mutex_;
  std::condition_variable invoke_cv_;
};

// Get the current thread's SocketThread (if any)
SocketThread* CurrentSocketThread();

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_SOCKET_THREAD_H */
