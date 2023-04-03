/*
 * count_down_latch.cc
 * Copyright (C) 2021 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "count_down_latch.h"

namespace avp {

CountDownLatch::CountDownLatch(int count) : count_(count) {}

void CountDownLatch::Wait() {
  std::unique_lock<std::mutex> l(mutex_);
  while (count_ > 0) {
    condition_.wait(l);
  }
}

void CountDownLatch::CountDown() {
  std::unique_lock<std::mutex> l(mutex_);
  --count_;
  if (count_ == 0) {
    condition_.notify_all();
  }
}

int CountDownLatch::GetCount() const {
  std::lock_guard<std::mutex> l(mutex_);
  return count_;
}

}  // namespace avp
