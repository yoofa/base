/*
 * task_runner_base.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef TASK_RUNNER_BASE_H
#define TASK_RUNNER_BASE_H

#include <functional>

#include "task.h"

namespace base {
class TaskRunnerBase {
 public:
  virtual void destruct() = 0;

  // post a task to be run, return true if the task maybe run at some point int
  // the future, and false the task will not be run.
  virtual void postTask(std::unique_ptr<Task> task) = 0;

  virtual void postDelayedTask(std::unique_ptr<Task> task, uint64_t timeUs) = 0;

  // virtual bool postTaskAndReplay(const Task& task, const Task& reply);

 protected:
  virtual ~TaskRunnerBase() = default;
};

struct TaskRunnerDeleter {
  void operator()(TaskRunnerBase* task_runner) const {
    task_runner->destruct();
  }
};

}  // namespace base

#endif /* !TASK_RUNNER_BASE_H */
