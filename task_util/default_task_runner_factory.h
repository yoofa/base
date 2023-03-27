/*
 * default_task_runner_factory.h
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef DEFAULT_TASK_RUNNER_FACTORY_H
#define DEFAULT_TASK_RUNNER_FACTORY_H

#include <memory>

#include "base/task_util/task_runner_factory.h"

namespace base {
std::unique_ptr<TaskRunnerFactory> CreateDefaultTaskRunnerFactory();

}  // namespace base

#endif /* !DEFAULT_TASK_RUNNER_FACTORY_H */
