/*
 * task_queue_unittest.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "base/count_down_latch.h"
#include "base/logging.h"
#include "base/task_util/default_task_runner_factory.h"
#include "base/task_util/task.h"
#include "base/task_util/task_runner.h"
#include "base/task_util/task_runner_base.h"
#include "base/task_util/task_runner_factory.h"
#include "base/test/task_runner_unittest.h"
#include "gtest/gtest-param-test.h"
#include "gtest/gtest.h"
#include "gtest/internal/gtest-param-util.h"
#include "test/gtest.h"

using namespace std::chrono_literals;

namespace avp {
using namespace base;

namespace {
INSTANTIATE_TEST_SUITE_P(Default,
                         TaskRunnerTest,
                         ::testing::Values(CreateDefaultTaskRunnerFactory));
}

std::unique_ptr<TaskRunnerBase, TaskRunnerDeleter> CreateTaskRunner(
    const std::unique_ptr<TaskRunnerFactory>& factory,
    std::string name,
    TaskRunnerFactory::Priority priority =
        TaskRunnerFactory::Priority::NORMAL) {
  return factory->CreateTaskRunner(name.c_str(), priority);
}

TEST_P(TaskRunnerTest, PostATask) {
  std::unique_ptr<TaskRunnerFactory> factory = GetParam()();
  auto runner =
      std::make_unique<TaskRunner>(CreateTaskRunner(factory, "PostATask"));
  std::mutex mutex;
  std::condition_variable cv;
  class CustomTask : public Task {
   public:
    explicit CustomTask(std::condition_variable* cv) : cv_(cv) {}

   private:
    bool run() override {
      cv_->notify_one();
      return true;
    }

    std::condition_variable* cv_;
  };
  runner->postTask(std::make_unique<CustomTask>(&cv));
  std::unique_lock<std::mutex> l(mutex);
  EXPECT_TRUE(cv.wait_for(l, 1000ms) == std::cv_status::no_timeout);

  runner->postTask([&cv]() { cv.notify_one(); });
  EXPECT_TRUE(cv.wait_for(l, 1000ms) == std::cv_status::no_timeout);
}

TEST_P(TaskRunnerTest, PostDelayedTask) {
  std::unique_ptr<TaskRunnerFactory> factory = GetParam()();
  auto runner = std::make_unique<TaskRunner>(
      CreateTaskRunner(factory, "PostDelayedTask"));
  std::mutex mutex;
  std::condition_variable cv;
  class CustomTask : public Task {
   public:
    explicit CustomTask(std::condition_variable* cv) : cv_(cv) {}

   private:
    bool run() override {
      cv_->notify_one();
      return true;
    }

    std::condition_variable* cv_;
  };
  runner->postDelayedTask(std::make_unique<CustomTask>(&cv), 100 * 1000);
  std::unique_lock<std::mutex> l(mutex);
  EXPECT_TRUE(cv.wait_for(l, 500ms) == std::cv_status::no_timeout);

  runner->postDelayedTask([&cv]() { cv.notify_one(); }, 100 * 1000);
  EXPECT_TRUE(cv.wait_for(l, 500ms) == std::cv_status::no_timeout);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TaskRunnerTest);
}  // namespace avp
