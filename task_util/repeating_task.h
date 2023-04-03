/*
 * repeating_task.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef REPEATING_TASK_H
#define REPEATING_TASK_H

#include <base/task_util/task.h>
#include <cstdint>
#include <memory>
#include "base/task_util/pending_task_flag.h"
#include "base/task_util/task_runner_base.h"

namespace avp {
using base::PendingTaskFlag;
using base::Task;
using base::TaskRunnerBase;

namespace repeating_task_impl {
class RepeatingTaskBase : public Task {
 public:
  RepeatingTaskBase(TaskRunnerBase* task_runner,
                    uint64_t first_delay_us,
                    std::shared_ptr<PendingTaskFlag> alive_flag);
  ~RepeatingTaskBase() override;

 private:
  virtual uint64_t RunClosure() = 0;

  bool Run() final;

  TaskRunnerBase* const task_runner_;
  uint64_t next_run_time_;
  std::shared_ptr<PendingTaskFlag> alive_flag_;
};

template <class Closure>
class RepeatingTaskImpl final : public RepeatingTaskBase {
 public:
  RepeatingTaskImpl(TaskRunnerBase* task_runner,
                    uint64_t first_delay,
                    Closure&& closure,
                    std::shared_ptr<PendingTaskFlag> alive_flag)
      : RepeatingTaskBase(task_runner, first_delay, std::move(alive_flag)),
        closure_(std::forward<Closure>(closure)) {
    static_assert(
        std::is_same<uint64_t,
                     typename std::result_of<decltype (&Closure::operator())(
                         Closure)>::type>::value,
        "");
  }

 private:
  uint64_t RunClosure() override { return closure_(); }

  typename std::remove_const<
      typename std::remove_reference<Closure>::type>::type closure_;
};

}  // namespace repeating_task_impl
class RepeatingTaskHandle {
 public:
  RepeatingTaskHandle() = default;
  ~RepeatingTaskHandle() = default;
  RepeatingTaskHandle(RepeatingTaskHandle&& other) = default;
  RepeatingTaskHandle& operator=(RepeatingTaskHandle&& other) = default;
  RepeatingTaskHandle(const RepeatingTaskHandle&) = delete;
  RepeatingTaskHandle& operator=(const RepeatingTaskHandle&) = delete;

  template <class Closure>
  static RepeatingTaskHandle Start(TaskRunnerBase* task_runner,
                                   Closure&& closure) {
    auto alive_flag = PendingTaskFlag::Create();
    task_runner->PostTask(
        std::make_unique<repeating_task_impl::RepeatingTaskImpl<Closure>>(
            task_runner, 0LL, std::forward<Closure>(closure), alive_flag));
    return RepeatingTaskHandle(std::move(alive_flag));
  }

  // DelayedStart is equivalent to Start except that the first invocation of the
  // closure will be delayed by the given amount.
  template <class Closure>
  static RepeatingTaskHandle DelayedStart(TaskRunnerBase* task_runner,
                                          uint64_t first_delay_us,
                                          Closure&& closure) {
    auto alive_flag = PendingTaskFlag::Create();
    task_runner->PostDelayedTask(
        std::make_unique<repeating_task_impl::RepeatingTaskImpl<Closure>>(
            task_runner, first_delay_us, std::forward<Closure>(closure),
            alive_flag),
        first_delay_us);
    return RepeatingTaskHandle(std::move(alive_flag));
  }

  void Stop();

  bool Running() const;

 private:
  explicit RepeatingTaskHandle(std::shared_ptr<PendingTaskFlag> alive_flag)
      : repeating_task_(std::move(alive_flag)) {}
  std::shared_ptr<PendingTaskFlag> repeating_task_;
};

}  // namespace avp

#endif /* !REPEATING_TASK_H */
