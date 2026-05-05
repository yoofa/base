/*
 * chat_server.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * A simple UDP chat server demo using AsyncUDPSocket.
 * Usage: ./chat_server <port>
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "base/net/async_udp_socket.h"
#include "base/net/physical_socket_server.h"

// NOLINTNEXTLINE
using namespace ave::base::net;

class ChatServer : public sigslot::has_slots<> {
 public:
  ChatServer(PhysicalSocketServer* ss, uint16_t port)
      : socket_server_(ss), running_(true) {
    Socket* socket = ss->CreateSocket(AF_INET, SOCK_DGRAM);
    SocketAddress bind_addr("0.0.0.0", port);
    socket_ = AsyncUDPSocket::Create(socket, bind_addr);

    if (socket_) {
      socket_->SignalReadPacket.connect(this, &ChatServer::OnPacketReceived);
      printf("Chat server started on port %d\n", port);
      printf("Waiting for clients...\n\n");
    } else {
      printf("Failed to create server socket\n");
      running_ = false;
    }
  }

  ~ChatServer() override { delete socket_; }

  void Run() {
    while (running_) {
      socket_server_->Wait(100);
    }
  }

  void Stop() { running_ = false; }

 private:
  void OnPacketReceived(AsyncPacketSocket* socket [[maybe_unused]],
                        const uint8_t* data,
                        size_t size,
                        const SocketAddress& addr,
                        int64_t timestamp [[maybe_unused]]) {
    std::string message(reinterpret_cast<const char*>(data), size);
    std::string client_key = addr.ToString();

    // Check if this is a new client
    if (clients_.find(client_key) == clients_.end()) {
      clients_[client_key] = addr;
      printf("[SERVER] New client connected: %s\n", client_key.c_str());

      // Send welcome message
      std::string welcome = "Welcome to the chat room! You are: " + client_key;
      socket_->SendTo(welcome.c_str(), welcome.size(), addr);

      // Notify others
      std::string join_msg = "[" + client_key + "] joined the chat";
      BroadcastMessage(join_msg, client_key);
    }

    // Handle special commands
    if (message == "/quit") {
      std::string leave_msg = "[" + client_key + "] left the chat";
      clients_.erase(client_key);
      BroadcastMessage(leave_msg, "");
      printf("[SERVER] Client disconnected: %s\n", client_key.c_str());
      return;
    }

    if (message == "/list") {
      std::string list = "Online users:\n";
      for (const auto& client : clients_) {
        list += "  - " + client.first + "\n";
      }
      socket_->SendTo(list.c_str(), list.size(), addr);
      return;
    }

    // Broadcast message to all clients
    printf("[%s]: %s\n", client_key.c_str(), message.c_str());
    std::string broadcast = "[" + client_key + "]: " + message;
    BroadcastMessage(broadcast, client_key);
  }

  void BroadcastMessage(const std::string& message,
                        const std::string& exclude_client) {
    for (const auto& client : clients_) {
      if (client.first != exclude_client) {
        socket_->SendTo(message.c_str(), message.size(), client.second);
      }
    }
  }

  PhysicalSocketServer* socket_server_;
  AsyncUDPSocket* socket_;
  std::map<std::string, SocketAddress> clients_;
  bool running_;
};

int main(int argc, char* argv[]) {
  uint16_t port = 8888;
  if (argc > 1) {
    port = static_cast<uint16_t>(atoi(argv[1]));
  }

  PhysicalSocketServer socket_server;
  ChatServer server(&socket_server, port);
  server.Run();

  return 0;
}
