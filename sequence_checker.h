/*
 * sequence_checker.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef SEQUENCE_CHECKER_H
#define SEQUENCE_CHECKER_H

#include "base/checks.h"
#include "base/sequence_checker_internal.h"
#include "base/thread_annotation.h"

namespace avp {

// SequenceChecker is a helper class used to help verify that some methods
// of a class are called on the same task queue or thread. A
// SequenceChecker is bound to a a task queue if the object is
// created on a task queue, or a thread otherwise.
//
//
// Example:
// class MyClass {
//  public:
//   void Foo() {
//     AVP_DCHECK_RUN_ON(&sequence_checker_);
//     ... (do stuff) ...
//   }
//
//  private:
//   SequenceChecker sequence_checker_;
// }
//
// In Release mode, IsCurrent will always return true.
class CAPABILITY("SequenceChecker") SequenceChecker
#if AVP_DCHECK_IS_ON
    : public avp_sequence_checker_internal::SequenceCheckerImpl {
  using Impl = avp_sequence_checker_internal::SequenceCheckerImpl;
#else
    : public avp_sequence_checker_internal::SequenceCheckerDoNothing {
  using Impl = avp_sequence_checker_internal::SequenceCheckerDoNothing;
#endif
 public:
  // Returns true if sequence checker is attached to the current sequence.
  bool IsCurrent() const { return Impl::IsCurrent(); }
  // Detaches checker from sequence to which it is attached. Next attempt
  // to do a check with this checker will result in attaching this checker
  // to the sequence on which check was performed.
  void Detach() { Impl::Detach(); }
};

}  // namespace avp

// Document if a function expected to be called from same thread/task queue.
#define AVP_RUN_ON(x) REQUIRES(x)

#define AVP_DCHECK_RUN_ON(x)                                                   \
  avp::avp_sequence_checker_internal::SequenceCheckerScope seq_check_scope(x); \
  DCHECK((x)->IsCurrent())                                                     \
      << avp::avp_sequence_checker_internal::ExpectationToString(x)

#endif /* !SEQUENCE_CHECKER_H */
