/*
 * sequence_checker_internal.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "sequence_checker_internal.h"

#include <pthread.h>
#include <sstream>

namespace ave {
namespace base {
namespace ave_sequence_checker_internal {
namespace {
// On Mac, returns the label of the current dispatch queue; elsewhere, return
// null.
const void* GetSystemQueueRef() {
#if defined(AVE_MAC)
  return dispatch_runner_get_label(DISPATCH_CURRENT_runner_LABEL);
#else
  return nullptr;
#endif
}

}  // namespace

SequenceCheckerImpl::SequenceCheckerImpl()
    : attached_(true),
      valid_runner_(base::TaskRunnerBase::Current()),
      valid_system_runner_(GetSystemQueueRef()) {}

bool SequenceCheckerImpl::IsCurrent() const {
  const TaskRunnerBase* const current_runner = base::TaskRunnerBase::Current();
  const pthread_t current_thread = pthread_self();
  const void* const current_system_runner = GetSystemQueueRef();
  lock_guard scoped_lock(lock_);
  if (!attached_) {  // Previously detached.
    attached_ = true;
    valid_runner_ = current_runner;
    valid_system_runner_ = current_system_runner;
    return true;
  }
  if (valid_runner_ || current_runner) {
    return valid_runner_ == current_runner;
  }
  if (valid_system_runner_ && valid_system_runner_ == current_system_runner) {
    return true;
  }
  return pthread_equal(valid_thread_, current_thread);
}

void SequenceCheckerImpl::Detach() {
  lock_guard scoped_lock(lock_);
  attached_ = false;
  // We don't need to touch the other members here, they will be
  // reset on the next call to IsCurrent().
}

#if AVE_DCHECK_IS_ON
std::string SequenceCheckerImpl::ExpectationToString() const {
  const base::TaskRunnerBase* const current_runner =
      base::TaskRunnerBase::Current();
  const pthread_t current_thread = pthread_self();
  const void* const current_system_runner = GetSystemQueueRef();
  lock_guard scoped_lock(lock_);
  if (!attached_)
    return "Checker currently not attached.";

  std::stringstream message;
  message << "# Expected: TQ: " << valid_runner_
          << " SysQ: " << valid_system_runner_
          << " Thread: " << reinterpret_cast<const void*>(valid_thread_) << "\n"
          << "# Actual:   TQ: " << current_runner
          << " SysQ: " << current_system_runner
          << " Thread: " << reinterpret_cast<const void*>(current_thread)
          << "\n";

  if ((valid_runner_ || current_runner) && valid_runner_ != current_runner) {
    message << "TaskQueue doesn't match\n";
  } else if (valid_system_runner_ &&
             valid_system_runner_ != current_system_runner) {
    message << "System queue doesn't match\n";
  } else if (!pthread_equal(valid_thread_, current_thread)) {
    message << "Threads don't match\n";
  }

  return message.str();
}
#endif  // AVE_DCHECK_IS_ON

std::string ExpectationToString(const SequenceCheckerImpl* checker) {
#if AVE_DCHECK_IS_ON
  return checker->ExpectationToString();
#else
  return std::string();
#endif
}

}  // namespace ave_sequence_checker_internal
}  // namespace base
}  // namespace ave
