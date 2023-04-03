/*
 * Mutex.h
 * Copyright (C) 2021 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVP_MUTEX_H
#define AVP_MUTEX_H

#include <pthread.h>

#include "thread_annotation.h"
#include "types.h"

namespace avp {

class Condition;

class CAPABILITY("mutex") Mutex {
 public:
  Mutex() { pthread_mutex_init(&mutex_, nullptr); }
  virtual ~Mutex() { pthread_mutex_destroy(&mutex_); }

  status_t Lock() ACQUIRE() { return -pthread_mutex_lock(&mutex_); }
  status_t Unlock() RELEASE() { return -pthread_mutex_unlock(&mutex_); }

  status_t TryLock() TRY_ACQUIRE(0) { return -pthread_mutex_trylock(&mutex_); }

  class SCOPED_CAPABILITY LockGuard {
   public:
    inline explicit LockGuard(Mutex& mutex) ACQUIRE(mutex) : mLock(mutex) {
      mLock.Lock();
    }
    inline explicit LockGuard(Mutex* mutex) ACQUIRE(mutex) : mLock(*mutex) {
      mLock.Lock();
    }
    inline ~LockGuard() RELEASE() { mLock.Unlock(); }

   private:
    Mutex& mLock;
    LockGuard(const LockGuard&);
    LockGuard& operator=(const LockGuard&);
  };

 private:
  friend class Condition;

  Mutex(const Mutex&);
  Mutex& operator=(const Mutex&);

  pthread_mutex_t mutex_;
};

using lock_guard = Mutex::LockGuard;

}  // namespace avp

#endif /* !AVP_MUTEX_H */
