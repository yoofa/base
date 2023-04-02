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
#include "test/gtest.h"

#ifndef TASK_RUNNER_UNITTEST_H
#define TASK_RUNNER_UNITTEST_H

namespace avp {
class TaskRunnerTest
    : public ::testing::TestWithParam<
          std::function<std::unique_ptr<base::TaskRunnerFactory>()>> {};

}  // namespace avp

#endif /* !TASK_RUNNER_UNITTEST_H */
