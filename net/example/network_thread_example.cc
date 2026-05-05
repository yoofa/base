/*
 * network_thread_example.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Example showing how to use NetworkThread for async network I/O
 * without blocking your main thread.
 */

#include <chrono>
#include <cstdio>
#include <thread>

#include "base/net/async_udp_socket.h"
#include "base/net/network_thread.h"

// NOLINTNEXTLINE
using namespace ave::base::net;

// A simple UDP client that runs on NetworkThread
class UdpClient : public sigslot::has_slots<> {
 public:
  UdpClient(NetworkThread* network_thread, const SocketAddress& server_addr)
      : network_thread_(network_thread),
        server_addr_(server_addr),
        socket_(nullptr),
        message_count_(0) {}

  ~UdpClient() override {
    if (socket_) {
      // Must destroy socket on network thread
      network_thread_->PostTask([this]() {
        delete socket_;
        socket_ = nullptr;
      });
      // Wait a bit for task to complete
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  // Initialize (call from any thread)
  void Initialize() {
    network_thread_->PostTask([this]() { InitializeOnNetworkThread(); });
  }

  // Send message (call from any thread)
  void SendMessage(const std::string& message) {
    network_thread_->PostTask(
        [this, message]() { SendMessageOnNetworkThread(message); });
  }

  int32_t message_count() const { return message_count_; }

 private:
  void InitializeOnNetworkThread() {
    Socket* socket =
        network_thread_->socket_server()->CreateSocket(AF_INET, SOCK_DGRAM);
    SocketAddress bind_addr("0.0.0.0", 0);
    socket_ = AsyncUDPSocket::Create(socket, bind_addr);

    if (socket_) {
      socket_->SignalReadPacket.connect(this, &UdpClient::OnPacketReceived);
      printf("[Client] Initialized, local addr: %s\n",
             socket_->GetLocalAddress().ToString().c_str());
    }
  }

  void SendMessageOnNetworkThread(const std::string& message) {
    if (socket_) {
      socket_->SendTo(message.c_str(), message.size(), server_addr_);
      printf("[Client] Sent: %s\n", message.c_str());
    }
  }

  void OnPacketReceived(AsyncPacketSocket* socket [[maybe_unused]],
                        const uint8_t* data,
                        size_t size,
                        const SocketAddress& addr [[maybe_unused]],
                        int64_t timestamp [[maybe_unused]]) {
    std::string message(reinterpret_cast<const char*>(data), size);
    printf("[Client] Received: %s\n", message.c_str());
    message_count_++;
  }

  NetworkThread* network_thread_;
  SocketAddress server_addr_;
  AsyncUDPSocket* socket_;
  std::atomic<int32_t> message_count_;
};

// A simple echo server
class EchoServer : public sigslot::has_slots<> {
 public:
  EchoServer(NetworkThread* network_thread, uint16_t port)
      : network_thread_(network_thread), port_(port), socket_(nullptr) {}

  ~EchoServer() override {
    if (socket_) {
      network_thread_->PostTask([this]() {
        delete socket_;
        socket_ = nullptr;
      });
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  void Initialize() {
    network_thread_->PostTask([this]() { InitializeOnNetworkThread(); });
  }

 private:
  void InitializeOnNetworkThread() {
    Socket* socket =
        network_thread_->socket_server()->CreateSocket(AF_INET, SOCK_DGRAM);
    SocketAddress bind_addr("0.0.0.0", port_);
    socket_ = AsyncUDPSocket::Create(socket, bind_addr);

    if (socket_) {
      socket_->SignalReadPacket.connect(this, &EchoServer::OnPacketReceived);
      printf("[Server] Listening on port %d\n", port_);
    }
  }

  void OnPacketReceived(AsyncPacketSocket* socket [[maybe_unused]],
                        const uint8_t* data,
                        size_t size,
                        const SocketAddress& addr,
                        int64_t timestamp [[maybe_unused]]) {
    std::string message(reinterpret_cast<const char*>(data), size);
    printf("[Server] Received from %s: %s\n", addr.ToString().c_str(),
           message.c_str());

    // Echo back with prefix
    std::string reply = "Echo: " + message;
    socket_->SendTo(reply.c_str(), reply.size(), addr);
  }

  NetworkThread* network_thread_;
  uint16_t port_;
  AsyncUDPSocket* socket_;
};

int main() {
  printf("=== NetworkThread Example ===\n\n");

  // Create network thread
  NetworkThread network_thread;
  network_thread.Start();

  // Create server on network thread
  EchoServer server(&network_thread, 12345);
  server.Initialize();

  // Wait for server to initialize
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Create client on network thread
  SocketAddress server_addr("127.0.0.1", 12345);
  UdpClient client(&network_thread, server_addr);
  client.Initialize();

  // Wait for client to initialize
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  printf("\n--- Main thread sending messages ---\n\n");

  // Send messages from main thread (non-blocking!)
  for (int i = 1; i <= 5; i++) {
    std::string msg = "Hello " + std::to_string(i);
    client.SendMessage(msg);

    // Simulate main thread doing other work
    printf("[Main] Doing other work while network I/O happens...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  // Wait for all replies
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  printf("\n--- Results ---\n");
  printf("Messages sent: 5\n");
  printf("Replies received: %d\n", client.message_count());

  // Stop network thread
  printf("\nStopping network thread...\n");
  network_thread.Stop();

  printf("Done!\n");
  return 0;
}
