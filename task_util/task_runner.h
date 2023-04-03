/*
 * task_runner.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef TASK_RUNNER_H
#define TASK_RUNNER_H

#include <memory>

#include "base/constructor_magic.h"
#include "base/task_util/task.h"
#include "base/task_util/task_runner_base.h"
#include "base/task_util/to_task.h"

namespace base {
class TaskRunner {
 public:
  explicit TaskRunner(
      std::unique_ptr<TaskRunnerBase, TaskRunnerDeleter> task_runner);
  ~TaskRunner();

  // post a task to be run
  void postTask(std::unique_ptr<base::Task> task);

  void postDelayedTask(std::unique_ptr<base::Task> task, uint64_t timeUs);

  TaskRunnerBase* Get() { return impl_; }

  template <class Closure,
            typename std::enable_if<!std::is_convertible<
                Closure,
                std::unique_ptr<base::Task>>::value>::type* = nullptr>
  void postTask(Closure&& closure) {
    return postTask(base::toTask(std::forward<Closure>(closure)));
  }

  template <class Closure,
            typename std::enable_if<!std::is_convertible<
                Closure,
                std::unique_ptr<base::Task>>::value>::type* = nullptr>
  void postDelayedTask(Closure&& closure, uint64_t timeUs) {
    return postDelayedTask(base::toTask(std::forward<Closure>(closure)),
                           timeUs);
  }

 protected:
 private:
  TaskRunnerBase* const impl_;

  AVP_DISALLOW_COPY_AND_ASSIGN(TaskRunner);
};

}  // namespace base

#endif /* !TASK_RUNNER_H */
