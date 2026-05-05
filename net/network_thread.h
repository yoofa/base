/*
 * network_thread.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * NetworkThread wraps SocketServer in a dedicated thread,
 * allowing async network I/O without blocking your main thread.
 */

#ifndef BASE_NET_NETWORK_THREAD_H
#define BASE_NET_NETWORK_THREAD_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "base/net/physical_socket_server.h"

namespace ave {
namespace base {
namespace net {

// NetworkThread runs a SocketServer in a dedicated thread.
// You can post tasks to run on the network thread.
class NetworkThread {
 public:
  NetworkThread();
  ~NetworkThread();

  // Disallow copy
  NetworkThread(const NetworkThread&) = delete;
  NetworkThread& operator=(const NetworkThread&) = delete;

  // Start the network thread
  void Start();

  // Stop the network thread (blocks until thread exits)
  void Stop();

  // Returns true if running on the network thread
  bool IsCurrent() const;

  // Post a task to run on the network thread
  void PostTask(std::function<void()> task);

  // Get the underlying SocketServer (use from network thread only)
  PhysicalSocketServer* socket_server() { return socket_server_.get(); }

  // Create a socket (can be called from any thread, but socket
  // should only be used from network thread)
  Socket* CreateSocket(int32_t family, int32_t type);

 private:
  void Run();
  void ProcessTasks();

  std::unique_ptr<PhysicalSocketServer> socket_server_;
  std::thread thread_;
  std::thread::id thread_id_;
  std::atomic<bool> running_;

  std::mutex task_mutex_;
  std::queue<std::function<void()>> tasks_;
};

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_NETWORK_THREAD_H */
