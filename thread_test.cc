/*
 * thread_test.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <unistd.h>

#include <iostream>

#include "thread.h"

// simple thread test
int main() {
  ave::base::Thread mythread(
      []() {
        for (int i = 0; i < 10; i++) {
          std::cout << "thread loop " << i << std::endl;
          sleep(1);
        }
      },
      "my");
  mythread.start();
  mythread.join();
}
