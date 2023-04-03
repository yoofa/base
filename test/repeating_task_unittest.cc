/*
 * repeating_task_unittest.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/task_util/repeating_task.h"
#include <unistd.h>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include "base/logging.h"
#include "base/task_util/task_runner_base.h"
#include "base/test/task_runner_for_test.h"
#include "gtest/gtest.h"
#include "test/gtest.h"

namespace avp {

TEST(RepeatingTaskTest, Example) {
  base::TaskRunnerForTest task_runner("TestRunner");
  std::mutex m;
  std::condition_variable cv;
  std::unique_lock<std::mutex> l(m);
  int i = 0;

  RepeatingTaskHandle::Start(task_runner.Get(), [&i, &cv]() {
    // LOG(LS_INFO) << "RepeatingTaskTest";
    if (i++ == 100) {
      cv.notify_one();
    }
    return (uint64_t)10 * 1000;
  });
  cv.wait(l);
  EXPECT_TRUE(i == 101);
}
}  // namespace avp
