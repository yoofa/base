/*
 * checks.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_CHECKS_H
#define AVE_CHECKS_H

#include <cstdlib>
#include <iostream>

#include "base/constructor_magic.h"
#include "base/logging.h"
#include "base/numerics/safe_compare.h"

#define AVE_CHECK(x) \
  if (!(x))          \
  LogMessageFatal().stream() << "Check failed: " #x

#define AVE_CHECK_OP(name, op, val1, val2)                               \
  if (!::ave::base::Safe##name((val1), (val2)))                          \
  LogMessageFatal().stream() << "Check failed: " #val1 " " #op " " #val2 \
                             << " (" << (val1) << " vs " << (val2) << ") "

#define AVE_CHECK_EQ(val1, val2) AVE_CHECK_OP(Eq, ==, val1, val2)
#define AVE_CHECK_NE(val1, val2) AVE_CHECK_OP(Ne, !=, val1, val2)
#define AVE_CHECK_LE(val1, val2) AVE_CHECK_OP(Le, <=, val1, val2)
#define AVE_CHECK_LT(val1, val2) AVE_CHECK_OP(Lt, <, val1, val2)
#define AVE_CHECK_GE(val1, val2) AVE_CHECK_OP(Ge, >=, val1, val2)
#define AVE_CHECK_GT(val1, val2) AVE_CHECK_OP(Gt, >, val1, val2)

#ifndef NDEBUG
#define AVE_DCHECK(x) AVE_CHECK(x)
#define AVE_DCHECK_EQ(x, y) AVE_CHECK_EQ(x, y)
#define AVE_DCHECK_NE(x, y) AVE_CHECK_NE(x, y)
#define AVE_DCHECK_LE(x, y) AVE_CHECK_LE(x, y)
#define AVE_DCHECK_LT(x, y) AVE_CHECK_LT(x, y)
#define AVE_DCHECK_GE(x, y) AVE_CHECK_GE(x, y)
#define AVE_DCHECK_GT(x, y) AVE_CHECK_GT(x, y)
#else  // NDEBUG

#define AVE_DCHECK(condition) \
  while (false)               \
  AVE_CHECK(condition)

#define AVE_DCHECK_EQ(val1, val2) \
  while (false)                   \
  AVE_CHECK_EQ(val1, val2)

#define AVE_DCHECK_NE(val1, val2) \
  while (false)                   \
  AVE_CHECK_NE(val1, val2)

#define AVE_DCHECK_LE(val1, val2) \
  while (false)                   \
  AVE_CHECK_LE(val1, val2)

#define AVE_DCHECK_LT(val1, val2) \
  while (false)                   \
  AVE_CHECK_LT(val1, val2)

#define AVE_DCHECK_GE(val1, val2) \
  while (false)                   \
  AVE_CHECK_GE(val1, val2)

#define AVE_DCHECK_GT(val1, val2) \
  while (false)                   \
  AVE_CHECK_GT(val1, val2)

#define AVE_DCHECK_STREQ(str1, str2) \
  while (false)                      \
  AVE_CHECK_STREQ(str1, str2)

#endif

#define AVE_UNREACHABLE_CODE_HIT false
#define AVE_NOTREACHED() AVE_DCHECK(AVE_UNREACHABLE_CODE_HIT)

class LogMessageFatal {
 public:
  LogMessageFatal() : log_message_(__FILE__, __LINE__, ::ave::base::LS_ERROR) {}

  ~LogMessageFatal() {
    // abort leter, no double free
    log_message_.~LogMessage();
    std::abort();
  }

  std::ostream& stream() { return log_message_.stream(); }

 private:
  ::ave::base::LogMessage log_message_;
  AVE_DISALLOW_COPY_AND_ASSIGN(LogMessageFatal);
};

#endif /* !CHECKS_H */
