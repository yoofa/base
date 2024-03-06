/*
 * task_runner_for_test.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "task_runner_for_test.h"

#include "base/task_util/default_task_runner_factory.h"

namespace avp {
namespace base {

TaskRunnerForTest::TaskRunnerForTest(std::string name,
                                     TaskRunnerFactory::Priority priority)
    : TaskRunner(
          CreateDefaultTaskRunnerFactory()->CreateTaskRunner(name.c_str(),
                                                             priority)) {}

}  // namespace base
}  // namespace avp
