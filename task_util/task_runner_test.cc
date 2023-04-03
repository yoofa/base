/*
 * task_runner_test.cc
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <unistd.h>

#include <base/logging.h>
#include "base/task_util/default_task_runner_factory.h"
#include "base/task_util/task_runner.h"

// simple task runner test
int main() {
  std::unique_ptr<base::TaskRunnerFactory> factory =
      base::CreateDefaultTaskRunnerFactory();
  std::unique_ptr<base::TaskRunner> runner =
      std::make_unique<base::TaskRunner>(factory->CreateTaskRunner(
          "task_runner_test", base::TaskRunnerFactory::Priority::NORMAL));
  LOG(avp::LS_INFO) << "main thread";

  runner->PostTask([]() { LOG(avp::LS_INFO) << "test post task"; });
  runner->PostDelayedTask(
      []() { LOG(avp::LS_INFO) << "test post delayed task"; }, 100 * 1000);
  sleep(2);
}
