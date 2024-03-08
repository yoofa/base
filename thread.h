/*
 * thread.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <functional>
#include <string>

#include "base/constructor_magic.h"
#include "base/count_down_latch.h"
#include "base/thread_defs.h"

namespace ave {
namespace base {

class Thread {
 public:
  typedef std::function<void()> ThreadFunc;
  explicit Thread(ThreadFunc,
                  const std::string& name = std::string(),
                  int priority = AVE_PRIORITY_DEFAULT);
  virtual ~Thread();

  void start(bool async = false);
  int join();

  bool started() { return started_; }

  pid_t tid() const { return tid_; }
  const std::string& name() const { return name_; }

 private:
  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  int priority_;
  CountDownLatch latch_;

  AVE_DISALLOW_COPY_AND_ASSIGN(Thread);
};

}  // namespace base
}  // namespace ave
#endif /* !THREAD_H */
