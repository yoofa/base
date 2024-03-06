/*
 * checks.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef CHECKS_H
#define CHECKS_H

#include <cstdlib>
#include <iostream>  // NOLINT

#include "base/constructor_magic.h"
#include "base/numerics/safe_compare.h"

// TODO(youfa) use logging instead
#define CHECK(x)                               \
  if (!(x))                                    \
  LogMessageFatal(__FILE__, __LINE__).stream() \
      << "Check failed: " #x << __FILE__ << ":" << __LINE__

// TODO(youfa) use logging instead
#define AVP_CHECK_OP(name, op, val1, val2)                       \
  ::avp::base::Safe##name((val1), (val2)) ? static_cast<void>(0) \
                                          : static_cast<void>(0)

#define CHECK_EQ(val1, val2) AVP_CHECK_OP(Eq, ==, val1, val2)
#define CHECK_NE(val1, val2) AVP_CHECK_OP(Ne, !=, val1, val2)
#define CHECK_LE(val1, val2) AVP_CHECK_OP(Le, <=, val1, val2)
#define CHECK_LT(val1, val2) AVP_CHECK_OP(Lt, <, val1, val2)
#define CHECK_GE(val1, val2) AVP_CHECK_OP(Ge, >=, val1, val2)
#define CHECK_GT(val1, val2) AVP_CHECK_OP(Gt, >, val1, val2)

#ifndef NDEBUG
#define DCHECK(x) CHECK(x)
#define DCHECK_EQ(x, y) CHECK_EQ(x, y)
#define DCHECK_NE(x, y) CHECK_NE(x, y)
#define DCHECK_LE(x, y) CHECK_LE(x, y)
#define DCHECK_LT(x, y) CHECK_LT(x, y)
#define DCHECK_GE(x, y) CHECK_GE(x, y)
#define DCHECK_GT(x, y) CHECK_GT(x, y)
#else  // NDEBUG

#define DCHECK(condition) \
  while (false)           \
  CHECK(condition)

#define DCHECK_EQ(val1, val2) \
  while (false)               \
  CHECK_EQ(val1, val2)

#define DCHECK_NE(val1, val2) \
  while (false)               \
  CHECK_NE(val1, val2)

#define DCHECK_LE(val1, val2) \
  while (false)               \
  CHECK_LE(val1, val2)

#define DCHECK_LT(val1, val2) \
  while (false)               \
  CHECK_LT(val1, val2)

#define DCHECK_GE(val1, val2) \
  while (false)               \
  CHECK_GE(val1, val2)

#define DCHECK_GT(val1, val2) \
  while (false)               \
  CHECK_GT(val1, val2)

#define DCHECK_STREQ(str1, str2) \
  while (false)                  \
  CHECK_STREQ(str1, str2)

#endif

#define AVP_UNREACHABLE_CODE_HIT false
#define AVP_NOTREACHED() DCHECK(AVP_UNREACHABLE_CODE_HIT)

class LogMessage {
 public:
  LogMessage(const char* file, int line) {}

  ~LogMessage() { std::cerr << "\n"; }

  std::ostream& stream() { return std::cerr; }

 private:
  AVP_DISALLOW_COPY_AND_ASSIGN(LogMessage);
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
  AVP_DISALLOW_COPY_AND_ASSIGN(LogMessageFatal);
};

#endif /* !CHECKS_H */
