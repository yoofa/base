/*
 * checks.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/checks.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#if defined(ANDROID)
#include <android/log.h>
#endif

namespace {

#if defined(__GNUC__)
__attribute__((__format__(__printf__, 2, 3)))
#endif
void AppendFormat(std::string* s, const char* fmt, ...) {
  va_list args, copy;
  va_start(args, fmt);
  va_copy(copy, args);
  const int predicted_length = std::vsnprintf(nullptr, 0, fmt, copy);
  va_end(copy);

  if (predicted_length > 0) {
    const size_t size = s->size();
    s->resize(size + predicted_length);
    // Pass "+ 1" to vsnprintf to include space for the '\0'.
    std::vsnprintf(&((*s)[size]), predicted_length + 1, fmt, args);
  }
  va_end(args);
}

}  // namespace

namespace ave {
namespace internal {

AVE_NORETURN void WriteFatalMessage(const char* file,
                                    int line,
                                    const std::string& msg) {
#if defined(ANDROID)
  __android_log_print(ANDROID_LOG_FATAL, "ave", "%s\n", msg.c_str());
#endif
  fprintf(stderr, "%s\n", msg.c_str());
  fflush(stderr);
#if defined(_WIN32)
  __debugbreak();
#endif
  abort();
}

#if AVE_CHECK_MSG_ENABLED
bool ParseArg(va_list* args, const CheckArgType** fmt, std::string* s) {
  if (**fmt == CheckArgType::kEnd) {
    return false;
  }

  switch (**fmt) {
    case CheckArgType::kInt:
      AppendFormat(s, "%d", va_arg(*args, int));
      break;
    case CheckArgType::kLong:
      AppendFormat(s, "%ld", va_arg(*args, long));
      break;
    case CheckArgType::kLongLong:
      AppendFormat(s, "%lld", va_arg(*args, long long));
      break;
    case CheckArgType::kUInt:
      AppendFormat(s, "%u", va_arg(*args, unsigned));
      break;
    case CheckArgType::kULong:
      AppendFormat(s, "%lu", va_arg(*args, unsigned long));
      break;
    case CheckArgType::kULongLong:
      AppendFormat(s, "%llu", va_arg(*args, unsigned long long));
      break;
    case CheckArgType::kDouble:
      AppendFormat(s, "%g", va_arg(*args, double));
      break;
    case CheckArgType::kLongDouble:
      AppendFormat(s, "%Lg", va_arg(*args, long double));
      break;
    case CheckArgType::kCharP:
      s->append(va_arg(*args, const char*));
      break;
    case CheckArgType::kStdString:
      s->append(*va_arg(*args, const std::string*));
      break;
    case CheckArgType::kVoidP:
      AppendFormat(s, "%p", va_arg(*args, const void*));
      break;
    default:
      s->append("[Invalid CheckArgType]");
      return false;
  }
  (*fmt)++;
  return true;
}

AVE_NORETURN void FatalLog(const char* file,
                           int line,
                           const char* message,
                           const CheckArgType* fmt,
                           ...) {
  va_list args;
  va_start(args, fmt);

  std::string s;
  AppendFormat(&s,
               "\n\n"
               "#\n"
               "# Fatal error in: %s, line %d\n"
               "# Check failed: %s",
               file, line, message);

  if (*fmt == CheckArgType::kCheckOp) {
    fmt++;
    std::string s1, s2;
    if (ParseArg(&args, &fmt, &s1) && ParseArg(&args, &fmt, &s2)) {
      AppendFormat(&s, " (%s vs. %s)\n# ", s1.c_str(), s2.c_str());
    }
  } else {
    s.append("\n# ");
  }

  while (ParseArg(&args, &fmt, &s))
    ;

  va_end(args);
  WriteFatalMessage(file, line, s);
}
#else
AVE_NORETURN void FatalLog(const char* file, int line) {
  std::string s;
  AppendFormat(&s,
               "\n\n"
               "#\n"
               "# Fatal error in: %s, line %d\n"
               "# Check failed.\n"
               "# ",
               file, line);
  WriteFatalMessage(file, line, s);
}
#endif  // AVE_CHECK_MSG_ENABLED

}  // namespace internal
}  // namespace ave