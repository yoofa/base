/*
 * thread.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "thread.h"

#include <assert.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <string>

namespace base {

namespace {

pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

struct ThreadData {
  typedef base::Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  std::string name_;
  pid_t* tid_;
  avp::CountDownLatch* latch_;

  ThreadData(ThreadFunc func,
             const std::string& name,
             pid_t* tid,
             avp::CountDownLatch* latch)
      : func_(std::move(func)), name_(name), tid_(tid), latch_(latch) {}

  void runInThread() {
    *tid_ = gettid();
    tid_ = nullptr;
    latch_->countDown();
    latch_ = nullptr;

    ::prctl(PR_SET_NAME, name_.empty() ? "thread" : name_.c_str());

    // exception ?
    func_();
  }
};

void* startThread(void* obj) {
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return nullptr;
}

}  // namespace

Thread::Thread(ThreadFunc func, const std::string& name, int priority)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(std::move(func)),
      name_(name),
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

  ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  struct sched_param sch_params;
  pthread_attr_getschedparam(&attr, &sch_params);
  sch_params.sched_priority = priority_;
  pthread_attr_setschedpolicy(&attr, SCHED_RR);
  pthread_attr_setschedparam(&attr, &sch_params);

  if (pthread_create(&pthreadId_, &attr, &startThread, data)) {
    started_ = false;
    delete data;
  } else {
    if (!async) {
      latch_.wait();
      assert(tid_ > 0);
    }
  }
}

int Thread::join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, nullptr);
}

}  // namespace base
