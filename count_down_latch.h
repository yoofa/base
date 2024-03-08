/*
 * count_down_latch.h
 * Copyright (C) 2021 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_COUNT_DOWN_LATCH_H
#define AVE_COUNT_DOWN_LATCH_H

#include <condition_variable>
#include <mutex>

#include "noncopyable.h"

namespace ave {
namespace base {

class CountDownLatch : noncopyable {
 public:
  explicit CountDownLatch(int count);

  void Wait();
  void CountDown();
  int GetCount() const;

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  int count_;
};

}  // namespace base
}  // namespace ave

#endif /* !AVE_COUNT_DOWN_LATCH_H */
