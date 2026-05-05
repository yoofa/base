/*
 * socket_thread_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/socket_thread.h"

#include <atomic>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace net {
namespace {

TEST(SocketThreadTest, StartStop) {
  SocketThread thread;
  EXPECT_FALSE(thread.IsCurrent());
  thread.Start();
  thread.Stop();
}

TEST(SocketThreadTest, PostTask) {
  SocketThread thread;
  thread.Start();

  std::atomic<bool> task_run{false};
  thread.PostTask([&task_run]() { task_run = true; });

  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  EXPECT_TRUE(task_run.load());
  thread.Stop();
}

TEST(SocketThreadTest, PostDelayedTask) {
  SocketThread thread;
  thread.Start();

  std::atomic<bool> task_run{false};
  auto start_time = std::chrono::steady_clock::now();

  thread.PostDelayedTask([&task_run]() { task_run = true; }, 50);

  EXPECT_FALSE(task_run.load());

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_TRUE(task_run.load());

  auto end_time = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_time - start_time)
                     .count();
  EXPECT_GE(elapsed, 50);

  thread.Stop();
}

TEST(SocketThreadTest, Invoke) {
  SocketThread thread;
  thread.Start();

  int result = 0;
  thread.Invoke([&result, &thread]() {
    EXPECT_TRUE(thread.IsCurrent());
    EXPECT_EQ(CurrentSocketThread(), &thread);
    result = 42;
  });

  EXPECT_EQ(result, 42);
  thread.Stop();
}

}  // namespace
}  // namespace net
}  // namespace base
}  // namespace ave
