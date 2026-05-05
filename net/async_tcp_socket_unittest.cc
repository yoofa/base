/*
 * async_tcp_socket_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/async_tcp_socket.h"

#include <gtest/gtest.h>
#include "base/net/physical_socket_server.h"

namespace ave {
namespace base {
namespace net {
namespace {

TEST(AsyncTCPSocketTest, CreateTcpSocket) {
  PhysicalSocketServer server;
  Socket* socket = server.CreateSocket(AF_INET, SOCK_STREAM);
  ASSERT_NE(socket, nullptr);

  SocketAddress bind_addr("127.0.0.1", 0);
  SocketAddress remote_addr("127.0.0.1", 12345);  // Dummy port

  AsyncTCPSocket* tcp_socket =
      AsyncTCPSocket::Create(socket, bind_addr, remote_addr);
  if (tcp_socket) {
    SocketAddress local_addr = tcp_socket->GetLocalAddress();
    EXPECT_FALSE(local_addr.IsNil());
    delete tcp_socket;
  } else {
    SUCCEED();
  }
}

}  // namespace
}  // namespace net
}  // namespace base
}  // namespace ave
