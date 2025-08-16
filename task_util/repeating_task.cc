/*
 * repeating_task.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "repeating_task.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

#include "base/logging.h"
#include "base/task_util/pending_task_flag.h"

namespace ave {
namespace base {

namespace repeating_task_impl {
static uint64_t GetNowUs() {
  auto systemClock = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(
             systemClock.time_since_epoch())
      .count();
}

RepeatingTaskBase::RepeatingTaskBase(TaskRunnerBase* task_runner,
                                     uint64_t first_delay_us,
                                     std::shared_ptr<PendingTaskFlag> alive)
    : task_runner_(task_runner),
      next_run_time_(GetNowUs() + first_delay_us),
      alive_flag_(std::move(alive)) {}
RepeatingTaskBase::~RepeatingTaskBase() = default;

bool RepeatingTaskBase::Run() {
  if (!alive_flag_->Alive()) {
    return true;
  }

  // TODO(youfa): use time utils to avoid overflow.
  uint64_t delay = RunClosure();

  uint64_t lost_time = GetNowUs() - next_run_time_;
  next_run_time_ += delay;
  delay = (lost_time > delay) ? 0 : delay;

  if (!alive_flag_->Alive()) {
    return true;
  }

  task_runner_->PostDelayedTask(std::unique_ptr<RepeatingTaskBase>(this),
                                delay);

  // Return false to tell the TaskQueue to not destruct this object since we
  // have taken ownership of it.
  return false;
}
}  // namespace repeating_task_impl

void RepeatingTaskHandle::Stop() {
  if (repeating_task_) {
    repeating_task_->SetNotAlive();
    repeating_task_ = nullptr;
  }
}

bool RepeatingTaskHandle::Running() const {
  return repeating_task_ != nullptr;
}

}  // namespace base
}  // namespace ave
