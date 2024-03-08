/*
 * task_runner_factory.h
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef TASK_RUNNER_FACTORY_H
#define TASK_RUNNER_FACTORY_H

#include <memory>
#include <string>

#include "base/task_util/task_runner_base.h"

namespace ave {
namespace base {
class TaskRunnerFactory {
 public:
  // map to thread priority
  enum class Priority { LOW = 0, NORMAL, HIGH };

  virtual ~TaskRunnerFactory() = default;

  virtual std::unique_ptr<TaskRunnerBase, TaskRunnerDeleter> CreateTaskRunner(
      const char* name,
      Priority priority) const = 0;
};

}  // namespace base
}  // namespace ave

#endif /* !TASK_RUNNER_FACTORY_H */
