/*
 * sequence_checker_internal.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef SEQUENCE_CHECKER_IMPL_H
#define SEQUENCE_CHECKER_IMPL_H

#include <string>

#include "base/mutex.h"
#include "base/task_util/task_runner_base.h"
#include "base/thread_annotation.h"

namespace avp {
namespace avp_sequence_checker_internal {

class SequenceCheckerImpl {
 public:
  SequenceCheckerImpl();
  ~SequenceCheckerImpl() = default;

  bool IsCurrent() const;

  void Detach();

  std::string ExpectationToString() const;

 private:
  mutable Mutex lock_;
  // These are mutable so that IsCurrent can set them.
  mutable bool attached_ GUARDED_BY(lock_);
  mutable pthread_t valid_thread_ GUARDED_BY(lock_);
  mutable const base::TaskRunnerBase* valid_runner_ GUARDED_BY(lock_);
  mutable const void* valid_system_runner_ GUARDED_BY(lock_);
};

// Do nothing implementation, for use in release mode.
//
// Note: You should almost always use the SequenceChecker class to get the
// right version for your build configuration.
class SequenceCheckerDoNothing {
 public:
  bool IsCurrent() const { return true; }
  void Detach() {}
};

// Helper class used by AVP_DCHECK_RUN_ON (see example usage below).
class SCOPED_CAPABILITY SequenceCheckerScope {
 public:
  template <typename ThreadLikeObject>
  explicit SequenceCheckerScope(const ThreadLikeObject* thread_like_object)
      ACQUIRE(thread_like_object) {}
  SequenceCheckerScope(const SequenceCheckerScope&) = delete;
  SequenceCheckerScope& operator=(const SequenceCheckerScope&) = delete;
  ~SequenceCheckerScope() RELEASE() {}

  template <typename ThreadLikeObject>
  static bool IsCurrent(const ThreadLikeObject* thread_like_object) {
    return thread_like_object->IsCurrent();
  }
};

std::string ExpectationToString(const SequenceCheckerImpl* checker);

// Catch-all implementation for types other than explicitly supported above.
template <typename ThreadLikeObject>
std::string ExpectationToString(const ThreadLikeObject*) {
  return std::string();
}

}  // namespace avp_sequence_checker_internal
}  // namespace avp

#endif /* !SEQUENCE_CHECKER_IMPL_H */
