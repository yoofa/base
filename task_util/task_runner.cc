/*
 * task_runner.cc
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "task_runner.h"

namespace ave {
namespace base {

TaskRunner::TaskRunner(
    std::unique_ptr<TaskRunnerBase, TaskRunnerDeleter> task_runner)
    : impl_(task_runner.release()) {}

TaskRunner::~TaskRunner() {
  impl_->Destruct();
}

bool TaskRunner::IsCurrent() const {
  return impl_->IsCurrent();
}

void TaskRunner::PostTask(std::unique_ptr<base::Task> task) {
  return impl_->PostTask(std::move(task));
}

void TaskRunner::PostTaskAndWait(std::unique_ptr<base::Task> task) {
  return impl_->PostDelayedTaskAndWait(std::move(task), 0LL, true);
}

void TaskRunner::PostDelayedTask(std::unique_ptr<base::Task> task,
                                 uint64_t time_us) {
  return impl_->PostDelayedTask(std::move(task), time_us);
}

void TaskRunner::PostDelayedTaskAndWait(std::unique_ptr<base::Task> task,
                                        uint64_t time_us) {
  return impl_->PostDelayedTaskAndWait(std::move(task), time_us, true);
}

}  // namespace base
}  // namespace ave
