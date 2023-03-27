/*
 * task_runner.cc
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "task_runner.h"

namespace base {

TaskRunner::TaskRunner(
    std::unique_ptr<TaskRunnerBase, TaskRunnerDeleter> task_runner)
    : impl_(task_runner.release()) {}

TaskRunner::~TaskRunner() {
  impl_->destruct();
}

void TaskRunner::postTask(std::unique_ptr<base::Task> task) {
  return impl_->postTask(std::move(task));
}

void TaskRunner::postDelayedTask(std::unique_ptr<base::Task> task,
                                 uint64_t timeUs) {
  return impl_->postDelayedTask(std::move(task), timeUs);
}

}  // namespace base
