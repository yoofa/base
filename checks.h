/*
 * checks.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_CHECKS_H
#define AVE_CHECKS_H

#include <cstdlib>
#include <iostream>  // NOLINT

#include "base/constructor_magic.h"
#include "base/numerics/safe_compare.h"

// TODO(youfa) use logging instead
#define AVE_CHECK(x)                           \
  if (!(x))                                    \
  LogMessageFatal(__FILE__, __LINE__).stream() \
      << "Check failed: " #x << __FILE__ << ":" << __LINE__

// TODO(youfa) use logging instead
#define AVE_CHECK_OP(name, op, val1, val2)                       \
  ::ave::base::Safe##name((val1), (val2)) ? static_cast<void>(0) \
                                          : static_cast<void>(0)

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

class LogMessage {
 public:
  LogMessage(const char* file, int line) {}

  ~LogMessage() { std::cerr << "\n"; }

  static std::ostream& stream() { return std::cerr; }

 private:
  AVE_DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

class LogMessageFatal : public LogMessage {
 public:
  LogMessageFatal(const char* file, int line)

      : LogMessage(file, line) {}

  ~LogMessageFatal() {
    std::cerr << "\n";
    std::abort();
  }

 private:
  AVE_DISALLOW_COPY_AND_ASSIGN(LogMessageFatal);
};

#endif /* !CHECKS_H */
