/*
 * task_runner_stdlib.cc
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "task_runner_stdlib.h"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "base/logging.h"
#include "base/task_util/task_runner_factory.h"
#include "base/thread.h"
#include "base/thread_defs.h"

namespace base {
namespace {

static uint64_t GetNowUs() {
  auto systemClock = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(
             systemClock.time_since_epoch())
      .count();
}

int TaskRunnerPriorityToStdlibPriority(TaskRunnerFactory::Priority priority) {
  switch (priority) {
    case TaskRunnerFactory::Priority::LOW:
      return AVP_PRIORITY_BACKGROUND;

    case TaskRunnerFactory::Priority::HIGH:
      return AVP_PRIORITY_AUDIO;

    case TaskRunnerFactory::Priority::NORMAL:
    default:
      return AVP_PRIORITY_NORMAL;
  }
}

}  // namespace

class TaskRunnerStdlib final : public TaskRunnerBase {
 public:
  TaskRunnerStdlib(const char* name, int priority);
  virtual ~TaskRunnerStdlib() override = default;

  void Destruct() override;
  void PostTask(std::unique_ptr<Task> task) override;
  void PostDelayedTask(std::unique_ptr<Task> task, uint64_t time_us) override;

 private:
  using OrderId = uint64_t;
  struct TaskEntry {
    uint64_t when_us_;
    OrderId order_{};
    std::unique_ptr<Task> task_;
  };

  struct TaskOrder {
    bool operator()(const std::unique_ptr<TaskEntry>& first,
                    const std::unique_ptr<TaskEntry>& second) const {
      return first->when_us_ > second->when_us_;
    }
  };

  struct NextTask {
    bool final_task_{false};
    std::unique_ptr<Task> run_task_;
  };

  std::string name_;
  int32_t priority_;
  std::unique_ptr<Thread> thread_;
  std::mutex mutex_;
  std::condition_variable task_condition_;
  bool need_quit_;
  OrderId task_order_id_;

  std::priority_queue<std::unique_ptr<TaskEntry>,
                      std::vector<std::unique_ptr<TaskEntry>>,
                      TaskOrder>
      task_queue_;

  bool Looping();
  void ProcessTask();
  AVP_DISALLOW_COPY_AND_ASSIGN(TaskRunnerStdlib);
};

TaskRunnerStdlib::TaskRunnerStdlib(const char* name, int priority)
    : name_(name),
      priority_(priority),
      thread_(std::make_unique<Thread>(
          [this] {
            CurrentTaskRunnerSetter set_current(this);
            ProcessTask();
          },
          name_,
          priority_)),
      need_quit_(false),
      task_order_id_(0LL) {
  thread_->start(false);
}

void TaskRunnerStdlib::Destruct() {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    need_quit_ = true;
  }
  task_condition_.notify_one();
}

void TaskRunnerStdlib::PostTask(std::unique_ptr<Task> task) {
  return PostDelayedTask(std::move(task), 0LL);
}

void TaskRunnerStdlib::PostDelayedTask(std::unique_ptr<Task> task,
                                       uint64_t delay_us) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (need_quit_) {
    return;
  }
  uint64_t when_us;
  if (delay_us > 0) {
    uint64_t now_us = GetNowUs();
    when_us = (delay_us > (std::numeric_limits<uint64_t>::max() - now_us)
                   ? std::numeric_limits<uint64_t>::max()
                   : (now_us + delay_us));
  } else {
    when_us = GetNowUs();
  }
  std::unique_ptr<TaskEntry> entry = std::make_unique<TaskEntry>();
  entry->when_us_ = when_us;
  entry->order_ = task_order_id_++;
  entry->task_ = std::move(task);
  task_queue_.push(std::move(entry));

  task_condition_.notify_one();
}

bool TaskRunnerStdlib::Looping() {
  std::lock_guard<std::mutex> guard(mutex_);
  return !need_quit_ || !task_queue_.empty();
}

void TaskRunnerStdlib::ProcessTask() {
  while (Looping()) {
    std::unique_ptr<Task> task;
    {
      std::unique_lock<std::mutex> l(mutex_);
      if (task_queue_.size() == 0) {
        task_condition_.wait(l);
        continue;
      }

      auto& entry = task_queue_.top();
      uint64_t now_ms = GetNowUs();
      if (entry->when_us_ > now_ms) {
        uint64_t delay_us = entry->when_us_ - now_ms;
        if (delay_us > std::numeric_limits<uint64_t>::max()) {
          delay_us = std::numeric_limits<uint64_t>::max();
        }
        task_condition_.wait_for(l, std::chrono::microseconds(delay_us));
        continue;
      }
      task = std::move(entry->task_);
      task_queue_.pop();
    }
    Task* release_ptr = task.release();
    // if return true , task runner take the ownership
    if (release_ptr->Run()) {
      delete release_ptr;
    }
    continue;
  }
}

class TaskRunnerStdlibFactory final : public TaskRunnerFactory {
 public:
  std::unique_ptr<TaskRunnerBase, TaskRunnerDeleter> CreateTaskRunner(
      const char* name,
      Priority priority) const override {
    return std::unique_ptr<TaskRunnerBase, TaskRunnerDeleter>(
        new TaskRunnerStdlib(name,
                             TaskRunnerPriorityToStdlibPriority(priority)));
  }
};

std::unique_ptr<TaskRunnerFactory> CreateTaskRunnerStdlibFactory() {
  return std::make_unique<TaskRunnerStdlibFactory>();
}

}  // namespace base
