/*
 * network_thread_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/network_thread.h"

#include <atomic>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace net {
namespace {

TEST(NetworkThreadTest, StartStop) {
  NetworkThread thread;
  EXPECT_FALSE(thread.IsCurrent());
  thread.Start();
  thread.Stop();
}

TEST(NetworkThreadTest, PostTask) {
  NetworkThread thread;
  thread.Start();

  std::atomic<bool> task_run{false};
  thread.PostTask([&task_run]() { task_run = true; });

  // Wait a bit
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  EXPECT_TRUE(task_run.load());
  thread.Stop();
}

TEST(NetworkThreadTest, CreateSocket) {
  NetworkThread thread;
  thread.Start();

  Socket* sock = thread.CreateSocket(AF_INET, SOCK_DGRAM);
  EXPECT_NE(sock, nullptr);
  delete sock;

  thread.Stop();
}

}  // namespace
}  // namespace net
}  // namespace base
}  // namespace ave
