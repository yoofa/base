/*
 * chat_client.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * A simple UDP chat client demo using AsyncUDPSocket.
 * Usage: ./chat_client <server_ip> <server_port>
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>

#include "base/net/async_udp_socket.h"
#include "base/net/physical_socket_server.h"

// NOLINTNEXTLINE
using namespace ave::base::net;

class ChatClient : public sigslot::has_slots<> {
 public:
  ChatClient(PhysicalSocketServer* ss,
             const std::string& server_ip,
             uint16_t server_port)
      : socket_server_(ss),
        server_addr_(server_ip, server_port),
        running_(true) {
    Socket* socket = ss->CreateSocket(AF_INET, SOCK_DGRAM);
    SocketAddress bind_addr("0.0.0.0", 0);  // Bind to any available port
    socket_ = AsyncUDPSocket::Create(socket, bind_addr);

    if (socket_) {
      socket_->SignalReadPacket.connect(this, &ChatClient::OnPacketReceived);
      printf("Connected to chat server at %s:%d\n", server_ip.c_str(),
             server_port);
      printf("Commands: /quit to exit, /list to see online users\n");
      printf("Type your message and press Enter to send.\n\n");

      // Send initial message to register with server
      std::string hello = "Hello";
      socket_->SendTo(hello.c_str(), hello.size(), server_addr_);
    } else {
      printf("Failed to create client socket\n");
      running_ = false;
    }
  }

  ~ChatClient() override { delete socket_; }

  void Run() {
    // Start input thread
    std::thread input_thread(&ChatClient::InputLoop, this);

    // Main event loop
    while (running_) {
      socket_server_->Wait(100);
    }

    input_thread.join();
  }

  void Stop() { running_ = false; }

 private:
  // NOLINTNEXTLINE
  void OnPacketReceived(AsyncPacketSocket* socket [[maybe_unused]],
                        const uint8_t* data,
                        size_t size,
                        const SocketAddress& addr [[maybe_unused]],
                        int64_t timestamp [[maybe_unused]]) {
    std::string message(reinterpret_cast<const char*>(data), size);
    printf("\r%s\n> ", message.c_str());
    fflush(stdout);
  }

  void InputLoop() {
    // NOLINTNEXTLINE
    char buffer[1024];
    while (running_) {
      printf("> ");
      fflush(stdout);

      if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
        break;
      }

      // Remove trailing newline
      size_t len = strlen(buffer);
      if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
      }

      if (len == 0) {
        continue;
      }

      std::string message(buffer);

      // Send message to server
      socket_->SendTo(message.c_str(), message.size(), server_addr_);

      // Check for quit command
      if (message == "/quit") {
        printf("Goodbye!\n");
        running_ = false;
        socket_server_->WakeUp();
        break;
      }
    }
  }

  PhysicalSocketServer* socket_server_;
  AsyncUDPSocket* socket_;
  SocketAddress server_addr_;
  bool running_;
};

int main(int argc, char* argv[]) {
  std::string server_ip = "127.0.0.1";
  uint16_t server_port = 8888;

  if (argc > 1) {
    server_ip = argv[1];
  }
  if (argc > 2) {
    server_port = static_cast<uint16_t>(atoi(argv[2]));
  }

  PhysicalSocketServer socket_server;
  ChatClient client(&socket_server, server_ip, server_port);
  client.Run();

  return 0;
}
