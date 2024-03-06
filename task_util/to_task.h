/*
 * to_task.h
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef TO_TASK_H
#define TO_TASK_H

#include <memory>
#include <utility>

#include "base/task_util/task.h"

namespace avp {
namespace base {

namespace base_closure_impl {
template <typename Closure>
class ClosureTask : public Task {
 public:
  explicit ClosureTask(Closure&& closure)
      : closure_(std::forward<Closure>(closure)) {}

 private:
  bool Run() override {
    closure_();
    return true;
  }
  typename std::decay<Closure>::type closure_;
};

template <typename Closure, typename Cleanup>
class ClosureTaskWithCleanup : public ClosureTask<Closure> {
 public:
  ClosureTaskWithCleanup(Closure&& closure, Cleanup&& cleanup)
      : cleanup_(std::forward<Cleanup>(cleanup)) {}
  ~ClosureTaskWithCleanup() override { cleanup_(); }

 private:
  typename std::decay<Cleanup>::type cleanup_;
};

}  // namespace base_closure_impl

template <typename Closure>
std::unique_ptr<Task> toTask(Closure&& closure) {
  return std::make_unique<base::base_closure_impl::ClosureTask<Closure>>(
      std::forward<Closure>(closure));
}

template <typename Closure, typename Cleanup>
std::unique_ptr<Task> toTask(Closure&& closure, Cleanup&& cleanup) {
  return std::make_unique<
      base::base_closure_impl::ClosureTaskWithCleanup<Closure, Cleanup>>(
      std::forward<Closure>(closure), std::forward<Cleanup>(cleanup));
}

}  // namespace base
}  // namespace avp

#endif /* !TO_TASK_H */
