/*
 * socket_thread_example.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Example demonstrating WebRTC-style SocketThread usage.
 */

#include <chrono>
#include <cstdio>
#include <thread>

#include "base/net/async_udp_socket.h"
#include "base/net/socket_thread.h"

// NOLINTNEXTLINE
using namespace ave::base::net;

//
// Example 1: Basic usage - network I/O on dedicated thread
//
void Example1_BasicUsage() {
  printf("\n=== Example 1: Basic SocketThread Usage ===\n\n");

  SocketThread network_thread;
  network_thread.Start();

  // Post task to create socket on network thread
  AsyncUDPSocket* socket = nullptr;
  network_thread.Invoke([&]() {
    Socket* s = network_thread.CreateSocket(AF_INET, SOCK_DGRAM);
    SocketAddress addr("0.0.0.0", 0);
    socket = AsyncUDPSocket::Create(s, addr);
    printf("[NetworkThread] Socket created at %s\n",
           socket->GetLocalAddress().ToString().c_str());
  });

  // Send data from main thread (via PostTask)
  network_thread.PostTask([socket]() {
    const char* msg = "Hello from network thread!";
    SocketAddress dest("127.0.0.1", 12345);
    socket->SendTo(msg, strlen(msg), dest);
    printf("[NetworkThread] Message sent\n");
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Cleanup
  network_thread.Invoke([socket]() { delete socket; });
  network_thread.Stop();

  printf("[Main] Done\n");
}

//
// Example 2: Invoke - synchronous call across threads
//
void Example2_Invoke() {
  printf("\n=== Example 2: Synchronous Invoke ===\n\n");

  SocketThread network_thread;
  network_thread.Start();

  int result = 0;

  // Invoke blocks until task completes on network thread
  printf("[Main] Calling Invoke()...\n");
  network_thread.Invoke([&]() {
    printf("[NetworkThread] Computing result...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    result = 42;
    printf("[NetworkThread] Done, result = %d\n", result);
  });

  printf("[Main] Invoke returned, result = %d\n", result);

  network_thread.Stop();
}

//
// Example 3: Delayed tasks
//
void Example3_DelayedTasks() {
  printf("\n=== Example 3: Delayed Tasks ===\n\n");

  SocketThread thread;
  thread.Start();

  auto start = std::chrono::steady_clock::now();

  // Post delayed tasks
  thread.PostDelayedTask(
      [start]() {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - start)
                           .count();
        printf("[Thread] Task 1 executed after %lld ms\n", elapsed);
      },
      300);

  thread.PostDelayedTask(
      [start]() {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - start)
                           .count();
        printf("[Thread] Task 2 executed after %lld ms\n", elapsed);
      },
      100);

  thread.PostDelayedTask(
      [start]() {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - start)
                           .count();
        printf("[Thread] Task 3 executed after %lld ms\n", elapsed);
      },
      200);

  // Wait for all tasks to complete
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  thread.Stop();
}

//
// Example 4: Wrap current thread (use main thread as SocketThread)
//
void Example4_WrapCurrentThread() {
  printf("\n=== Example 4: Wrap Current Thread ===\n\n");

  // Make current thread a SocketThread
  auto thread = SocketThread::WrapCurrent();

  // Now we can use network I/O on main thread
  Socket* socket = thread->CreateSocket(AF_INET, SOCK_DGRAM);
  SocketAddress addr("0.0.0.0", 0);
  auto* udp = AsyncUDPSocket::Create(socket, addr);

  printf("[Main] Socket created at %s\n",
         udp->GetLocalAddress().ToString().c_str());

  // Process events manually (non-blocking)
  for (int i = 0; i < 5; i++) {
    printf("[Main] Processing messages (iteration %d)...\n", i + 1);
    thread->ProcessMessages(100);
  }

  delete udp;
  thread->Stop();

  printf("[Main] Done\n");
}

//
// Example 5: Echo server/client on same thread (real usage pattern)
//
class EchoServer : public sigslot::has_slots<> {
 public:
  EchoServer(SocketThread* thread, uint16_t port)
      : thread_(thread), socket_(nullptr) {
    thread_->PostTask([this, port]() { Initialize(port); });
  }

  ~EchoServer() override {
    if (socket_) {
      thread_->Invoke([this]() { delete socket_; });
    }
  }

 private:
  void Initialize(uint16_t port) {
    Socket* s = thread_->CreateSocket(AF_INET, SOCK_DGRAM);
    SocketAddress addr("0.0.0.0", port);
    socket_ = AsyncUDPSocket::Create(s, addr);
    socket_->SignalReadPacket.connect(this, &EchoServer::OnPacket);
    printf("[Server] Listening on port %d\n", port);
  }

  void OnPacket(AsyncPacketSocket* socket [[maybe_unused]],
                const uint8_t* data,
                size_t size,
                const SocketAddress& addr,
                int64_t ts [[maybe_unused]]) {
    std::string msg(reinterpret_cast<const char*>(data), size);
    printf("[Server] Received: %s\n", msg.c_str());

    std::string reply = "Echo: " + msg;
    socket_->SendTo(reply.c_str(), reply.size(), addr);
  }

  SocketThread* thread_;
  AsyncUDPSocket* socket_;
};

class EchoClient : public sigslot::has_slots<> {
 public:
  EchoClient(SocketThread* thread, const SocketAddress& server)
      : thread_(thread), server_(server), socket_(nullptr), replies_(0) {
    thread_->PostTask([this]() { Initialize(); });
  }

  ~EchoClient() override {
    if (socket_) {
      thread_->Invoke([this]() { delete socket_; });
    }
  }

  void Send(const std::string& msg) {
    thread_->PostTask([this, msg]() {
      if (socket_) {
        socket_->SendTo(msg.c_str(), msg.size(), server_);
        printf("[Client] Sent: %s\n", msg.c_str());
      }
    });
  }

  int replies() const { return replies_; }

 private:
  void Initialize() {
    Socket* s = thread_->CreateSocket(AF_INET, SOCK_DGRAM);
    SocketAddress addr("0.0.0.0", 0);
    socket_ = AsyncUDPSocket::Create(s, addr);
    socket_->SignalReadPacket.connect(this, &EchoClient::OnPacket);
    printf("[Client] Ready\n");
  }

  void OnPacket(AsyncPacketSocket* socket [[maybe_unused]],
                const uint8_t* data,
                size_t size,
                const SocketAddress& addr [[maybe_unused]],
                int64_t ts [[maybe_unused]]) {
    std::string msg(reinterpret_cast<const char*>(data), size);
    printf("[Client] Received: %s\n", msg.c_str());
    replies_++;
  }

  SocketThread* thread_;
  SocketAddress server_;
  AsyncUDPSocket* socket_;
  std::atomic<int> replies_;
};

void Example5_EchoServerClient() {
  printf("\n=== Example 5: Echo Server/Client ===\n\n");

  SocketThread thread;
  thread.Start();

  // Create server
  EchoServer server(&thread, 22222);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // Create client
  SocketAddress server_addr("127.0.0.1", 22222);
  EchoClient client(&thread, server_addr);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // Send messages from main thread
  client.Send("Hello");
  client.Send("World");
  client.Send("Test");

  // Wait for replies
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  printf("\n[Main] Replies received: %d\n", client.replies());

  thread.Stop();
}

int main() {
  printf("====================================\n");
  printf("  WebRTC-Style SocketThread Demo\n");
  printf("====================================\n");

  Example1_BasicUsage();
  Example2_Invoke();
  Example3_DelayedTasks();
  Example4_WrapCurrentThread();
  Example5_EchoServerClient();

  printf("\n=== All examples completed! ===\n");
  return 0;
}
