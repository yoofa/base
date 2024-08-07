/*
 * task_runner_unittest.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <functional>
#include <memory>

#include "base/task_util/task_runner_factory.h"
#include "gtest/gtest.h"

#ifndef TASK_RUNNER_UNITTEST_H
#define TASK_RUNNER_UNITTEST_H

namespace ave {
namespace base {

class TaskRunnerTest
    : public ::testing::TestWithParam<
          std::function<std::unique_ptr<base::TaskRunnerFactory>()>> {};

}  // namespace base
}  // namespace ave

#endif /* !TASK_RUNNER_UNITTEST_H */
