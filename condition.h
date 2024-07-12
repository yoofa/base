/*
 * Condition.h
 * Copyright (C) 2021 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_CONDITION_H
#define AVE_CONDITION_H

#include <climits>
#include <cstdint>
#include <ctime>

#include "mutex.h"
#include "types.h"

namespace ave {

class Condition {
 public:
  Condition() { pthread_cond_init(&cond_, nullptr); }
  virtual ~Condition() { pthread_cond_destroy(&cond_); }

  status_t Wait(Mutex& mutex) {
    return -pthread_cond_wait(&cond_, &mutex.mutex_);
  }
  status_t WaitRelative(Mutex& mutex, nsecs_t reltime) {
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    int64_t reltime_sec = reltime / 1000000000;

    ts.tv_nsec += static_cast<long>(reltime % 1000000000);
    if (reltime_sec < INT64_MAX && ts.tv_nsec >= 1000000000) {
      ts.tv_nsec -= 1000000000;
      ++reltime_sec;
    }

    int64_t time_sec = ts.tv_sec;
    if (time_sec > INT64_MAX - reltime_sec) {
      time_sec = INT64_MAX;
    } else {
      time_sec += reltime_sec;
    }

    ts.tv_sec = (time_sec > LONG_MAX) ? LONG_MAX : static_cast<long>(time_sec);

    return -pthread_cond_timedwait(&cond_, &mutex.mutex_, &ts);
  }

  void Signal() { pthread_cond_signal(&cond_); }

  void Broadcast() { pthread_cond_broadcast(&cond_); }

 private:
  pthread_cond_t cond_{};
};

}  // namespace ave

#endif /* !AVE_CONDITION_H */
