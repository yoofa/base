/*
 * default_task_runner_factory_stdlib.cc
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "default_task_runner_factory.h"

#include "base/task_util/task_runner_stdlib.h"

namespace avp {
namespace base {

std::unique_ptr<TaskRunnerFactory> CreateDefaultTaskRunnerFactory() {
  return CreateTaskRunnerStdlibFactory();
}

}  // namespace base
}  // namespace avp
