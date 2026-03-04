/*
 * thread.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "thread.h"

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cassert>
#include <string>

namespace ave {
namespace base {
namespace {

pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

struct ThreadData {
  using ThreadFunc = base::Thread::ThreadFunc;
  ThreadFunc func_;
  std::string name_;
  pid_t* tid_;
  CountDownLatch* latch_;
  int priority_;

  ThreadData(ThreadFunc func,
             std::string name,
             pid_t* tid,
             CountDownLatch* latch,
             int priority)
      : func_(std::move(func)),
        name_(std::move(name)),
        tid_(tid),
        latch_(latch),
        priority_(priority) {}

  void runInThread() {
    *tid_ = gettid();
    tid_ = nullptr;
    latch_->CountDown();
    latch_ = nullptr;

    ::prctl(PR_SET_NAME, name_.empty() ? "thread" : name_.c_str());

    // Apply thread nice priority (lower value = higher priority)
    if (priority_ != 0) {
      setpriority(PRIO_PROCESS, 0, priority_);
    }

    func_();
  }
};

void* startThread(void* obj) {
  auto* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return nullptr;
}

}  // namespace

Thread::Thread(ThreadFunc func, std::string name, int priority, bool joinable)
    : started_(false),
      joined_(false),
      joinable_(joinable),
      pthreadId_(0),
      tid_(0),
      func_(std::move(func)),
      name_(std::move(name)),
      priority_(priority),
      latch_(1) {}

Thread::~Thread() {
  if (started_ && !joined_) {
    pthread_detach(pthreadId_);
  }
}

void Thread::start(bool async) {
  assert(!started_);
  started_ = true;

  auto* data = new ThreadData(func_, name_, &tid_, &latch_, priority_);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  // Set the stack size to 1M.
  pthread_attr_setstacksize(&attr, 1024 * 1024);
  pthread_attr_setdetachstate(
      &attr, joinable_ ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED);

  int err = pthread_create(&pthreadId_, &attr, &startThread, data);
  pthread_attr_destroy(&attr);

  if (err != 0) {
    started_ = false;
    delete data;
  }

  if (started_ && !async) {
    latch_.Wait();
    assert(tid_ > 0);
  }
}

int Thread::join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, nullptr);
}

}  // namespace base
}  // namespace ave
