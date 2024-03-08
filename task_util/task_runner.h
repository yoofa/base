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
#include "base/thread_annotation.h"

namespace ave {
namespace base {
class CAPABILITY("TaskRunner") TaskRunner {
 public:
  explicit TaskRunner(
      std::unique_ptr<TaskRunnerBase, TaskRunnerDeleter> task_runner);
  ~TaskRunner();

  // Used for AVE_DCHECKing the current runner.
  bool IsCurrent() const;

  // post a task to be run
  void PostTask(std::unique_ptr<base::Task> task);

  void PostDelayedTask(std::unique_ptr<base::Task> task, uint64_t time_us);

  TaskRunnerBase* Get() { return impl_; }

  template <class Closure,
            typename std::enable_if<!std::is_convertible<
                Closure,
                std::unique_ptr<base::Task>>::value>::type* = nullptr>
  void PostTask(Closure&& closure) {
    return PostTask(base::toTask(std::forward<Closure>(closure)));
  }

  template <class Closure,
            typename std::enable_if<!std::is_convertible<
                Closure,
                std::unique_ptr<base::Task>>::value>::type* = nullptr>
  void PostDelayedTask(Closure&& closure, uint64_t timeUs) {
    return PostDelayedTask(base::toTask(std::forward<Closure>(closure)),
                           timeUs);
  }

 protected:
 private:
  TaskRunnerBase* const impl_;

  AVE_DISALLOW_COPY_AND_ASSIGN(TaskRunner);
};

}  // namespace base
}  // namespace ave

#endif /* !TASK_RUNNER_H */
