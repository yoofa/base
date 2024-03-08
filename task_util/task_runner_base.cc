/*
 * task_runner_base.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "task_runner_base.h"

#include <pthread.h>

#include "base/checks.h"

namespace ave {
namespace base {
namespace {

pthread_key_t g_queue_ptr_tls = 0;

void InitializeTls() {
  AVE_CHECK(pthread_key_create(&g_queue_ptr_tls, nullptr) == 0);
}

pthread_key_t GetQueuePtrTls() {
  static pthread_once_t init_once = PTHREAD_ONCE_INIT;
  AVE_CHECK(pthread_once(&init_once, &InitializeTls) == 0);
  return g_queue_ptr_tls;
}

}  // namespace

TaskRunnerBase* TaskRunnerBase::Current() {
  return static_cast<TaskRunnerBase*>(pthread_getspecific(GetQueuePtrTls()));
}

TaskRunnerBase::CurrentTaskRunnerSetter::CurrentTaskRunnerSetter(
    TaskRunnerBase* task_queue)
    : previous_(TaskRunnerBase::Current()) {
  pthread_setspecific(GetQueuePtrTls(), task_queue);
}

TaskRunnerBase::CurrentTaskRunnerSetter::~CurrentTaskRunnerSetter() {
  pthread_setspecific(GetQueuePtrTls(), previous_);
}

}  // namespace base
}  // namespace ave
