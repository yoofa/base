/*
 * async_socket_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <cstring>

#include "base/net/async_udp_socket.h"
#include "base/net/physical_socket_server.h"
#include "gtest/gtest.h"

namespace ave {
namespace base {
namespace net {

// Helper class for receiving packets
class PacketReceiver : public sigslot::has_slots<> {
 public:
  PacketReceiver() : received_(false) {}

  void OnPacketReceived(AsyncPacketSocket* socket [[maybe_unused]],
                        const uint8_t* data,
                        size_t size,
                        const SocketAddress& addr,
                        int64_t timestamp [[maybe_unused]]) {
    received_ = true;
    received_data_ = std::string(reinterpret_cast<const char*>(data), size);
    received_from_ = addr;
  }

  bool received() const { return received_; }
  const std::string& received_data() const { return received_data_; }
  const SocketAddress& received_from() const { return received_from_; }

 private:
  bool received_;
  std::string received_data_;
  SocketAddress received_from_;
};

class AsyncSocketTest : public ::testing::Test {
 protected:
  void SetUp() override {
    socket_server_ = std::make_unique<PhysicalSocketServer>();
  }

  void TearDown() override { socket_server_.reset(); }

  std::unique_ptr<PhysicalSocketServer> socket_server_;
};

TEST_F(AsyncSocketTest, CreateUdpSocket) {
  Socket* socket = socket_server_->CreateSocket(AF_INET, SOCK_DGRAM);
  ASSERT_NE(socket, nullptr);

  SocketAddress bind_addr("127.0.0.1", 0);
  auto* udp_socket = AsyncUDPSocket::Create(socket, bind_addr);
  ASSERT_NE(udp_socket, nullptr);

  SocketAddress local_addr = udp_socket->GetLocalAddress();
  EXPECT_FALSE(local_addr.IsNil());
  EXPECT_NE(local_addr.port(), 0);

  delete udp_socket;
}

TEST_F(AsyncSocketTest, UdpLoopback) {
  // Create sender socket
  Socket* sender_sock = socket_server_->CreateSocket(AF_INET, SOCK_DGRAM);
  SocketAddress sender_addr("127.0.0.1", 0);
  auto* sender = AsyncUDPSocket::Create(sender_sock, sender_addr);
  ASSERT_NE(sender, nullptr);

  // Create receiver socket
  Socket* receiver_sock = socket_server_->CreateSocket(AF_INET, SOCK_DGRAM);
  SocketAddress receiver_addr("127.0.0.1", 0);
  auto* receiver = AsyncUDPSocket::Create(receiver_sock, receiver_addr);
  ASSERT_NE(receiver, nullptr);

  // Get actual receiver address after bind
  SocketAddress actual_receiver_addr = receiver->GetLocalAddress();

  // Create packet receiver helper
  PacketReceiver packet_receiver;
  receiver->SignalReadPacket.connect(&packet_receiver,
                                     &PacketReceiver::OnPacketReceived);

  // Send data
  const char* test_message = "Hello, UDP!";
  int sent =
      sender->SendTo(test_message, strlen(test_message), actual_receiver_addr);
  EXPECT_GT(sent, 0);

  // Wait for event
  for (int i = 0; i < 100 && !packet_receiver.received(); ++i) {
    socket_server_->Wait(10);
  }

  EXPECT_TRUE(packet_receiver.received());
  EXPECT_EQ(packet_receiver.received_data(), test_message);

  delete sender;
  delete receiver;
}

}  // namespace net
}  // namespace base
}  // namespace ave
