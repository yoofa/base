/*
 * task_runner_stdlib.h
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef TASK_RUNNER_STDLIB_H
#define TASK_RUNNER_STDLIB_H

#include <memory>

#include <base/task_util/task_runner_factory.h>

namespace avp {
namespace base {

std::unique_ptr<TaskRunnerFactory> CreateTaskRunnerStdlibFactory();

}
}  // namespace avp

#endif /* !TASK_RUNNER_STDLIB_H */
