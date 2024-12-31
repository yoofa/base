/*
 * checks_unittest.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/checks.h"

#include <gtest/gtest.h>

namespace ave {
namespace {

TEST(ChecksTest, ExpressionNotEvaluated) {
  int i = 0;
  AVE_CHECK(true) << "i=" << ++i;
  AVE_CHECK_EQ(i, 0) << "Previous check passed, but i was incremented!";
}

TEST(ChecksTest, CheckSucceeds) {
  AVE_CHECK(true);
  AVE_CHECK_EQ(1, 1);
  AVE_CHECK_NE(1, 2);
  AVE_CHECK_LE(1, 2);
  AVE_CHECK_LT(1, 2);
  AVE_CHECK_GE(2, 1);
  AVE_CHECK_GT(2, 1);
}

#if GTEST_HAS_DEATH_TEST && !defined(ANDROID)
TEST(ChecksDeathTest, Checks) {
#if AVE_CHECK_MSG_ENABLED
  EXPECT_DEATH(AVE_CHECK(false) << "Message",
               "\n\n"
               "#\n"
               "# Fatal error in: \\S+, line \\w+\n"
               "# Check failed: false\n"
               "# Message");

  int a = 1, b = 2;
  EXPECT_DEATH(AVE_CHECK_EQ(a, b) << 1 << 2u,
               "\n\n"
               "#\n"
               "# Fatal error in: \\S+, line \\w+\n"
               "# Check failed: a == b \\(1 vs. 2\\)\n"
               "# 12");
  AVE_CHECK_EQ(5, 5);

  AVE_CHECK(true) << "Shouldn't crash" << 1;
  EXPECT_DEATH(AVE_CHECK(false) << "Hi there!",
               "\n\n"
               "#\n"
               "# Fatal error in: \\S+, line \\w+\n"
               "# Check failed: false\n"
               "# Hi there!");
#else
  EXPECT_DEATH(AVE_CHECK(false) << "Message",
               "\n\n"
               "#\n"
               "# Fatal error in: \\S+, line \\w+\n"
               "# Check failed.\n"
               "# ");

  int a = 1, b = 2;
  EXPECT_DEATH(AVE_CHECK_EQ(a, b) << 1 << 2u,
               "\n\n"
               "#\n"
               "# Fatal error in: \\S+, line \\w+\n"
               "# Check failed.\n"
               "# ");
  AVE_CHECK_EQ(5, 5);

  AVE_CHECK(true) << "Shouldn't crash" << 1;
  EXPECT_DEATH(AVE_CHECK(false) << "Hi there!",
               "\n\n"
               "#\n"
               "# Fatal error in: \\S+, line \\w+\n"
               "# Check failed.\n"
               "# ");
#endif  // AVE_CHECK_MSG_ENABLED
}

TEST(ChecksDeathTest, DCheckFailsInDebug) {
#if AVE_DCHECK_IS_ON
  EXPECT_DEATH(AVE_DCHECK(false),
               "\n\n"
               "#\n"
               "# Fatal error in: \\S+, line \\w+\n"
               "# Check failed: false");
#else
  AVE_DCHECK(false);  // Should not crash in release
#endif
}

TEST(ChecksDeathTest, NotReached) {
  EXPECT_DEATH(AVE_NOTREACHED(), "");
}
#endif  // GTEST_HAS_DEATH_TEST && !defined(ANDROID)

// Test different types of arguments
TEST(ChecksTest, DifferentTypes) {
  const char* const str = "hello";
  std::string cpp_str = "world";
  std::string_view str_view = "view";
  void* ptr = nullptr;

  AVE_CHECK(true) << str;       // const char*
  AVE_CHECK(true) << cpp_str;   // std::string
  AVE_CHECK(true) << str_view;  // std::string_view
  AVE_CHECK(true) << ptr;       // void*
  AVE_CHECK(true) << 42;        // int
  AVE_CHECK(true) << 42u;       // unsigned
  AVE_CHECK(true) << 42.0;      // double
  AVE_CHECK(true) << 42.0f;     // float
}

}  // namespace
}  // namespace ave