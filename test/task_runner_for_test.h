/*
 * task_runner_for_test.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef TASK_RUNNER_FOR_TEST_H
#define TASK_RUNNER_FOR_TEST_H

#include <string>

#include "base/task_util/task_runner.h"
#include "base/task_util/task_runner_factory.h"

namespace ave {

namespace base {

class TaskRunnerForTest : public TaskRunner {
 public:
  TaskRunnerForTest(std::string name = "TestRunner",
                    TaskRunnerFactory::Priority priority =
                        TaskRunnerFactory::Priority::NORMAL);
  ~TaskRunnerForTest() = default;

 private:
};

}  // namespace base
}  // namespace ave

#endif /* !TASK_RUNNER_FOR_TEST_H */
