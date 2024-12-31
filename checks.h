/*
 * checks.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_BASE_CHECKS_H_
#define AVE_BASE_CHECKS_H_

// Debug build check control
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
#define AVE_DCHECK_IS_ON 1
#else
#define AVE_DCHECK_IS_ON 0
#endif

// Message control
#ifdef AVE_DISABLE_CHECK_MSG
#define AVE_CHECK_MSG_ENABLED 0
#else
#define AVE_CHECK_MSG_ENABLED 1
#endif

#include <string>
#include "attributes.h"
#include "numerics/safe_compare.h"

namespace ave {
namespace internal {

enum class CheckArgType : int8_t {
  kEnd = 0,
  kInt,
  kLong,
  kLongLong,
  kUInt,
  kULong,
  kULongLong,
  kDouble,
  kLongDouble,
  kCharP,
  kStdString,
  kStringView,
  kVoidP,
  // kCheckOp doesn't represent an argument type. Instead, it is sent as the
  // first argument from RTC_CHECK_OP to make FatalLog use the next two
  // arguments to build the special CHECK_OP error message
  // (the "a == b (1 vs. 2)" bit).
  kCheckOp,
};

AVE_NORETURN void WriteFatalMessage(const char* file,
                                    int line,
                                    const std::string& msg);

#if AVE_CHECK_MSG_ENABLED
AVE_NORETURN void FatalLog(const char* file,
                           int line,
                           const char* message,
                           const CheckArgType* fmt,
                           ...);
#else
AVE_NORETURN void FatalLog(const char* file, int line);
#endif

// Value wrapper for logging
template <CheckArgType N, typename T>
struct Val {
  static constexpr CheckArgType Type() { return N; }
  T GetVal() const { return val; }
  T val;
};

// String value wrapper
struct ToStringVal {
  static constexpr CheckArgType Type() { return CheckArgType::kStdString; }
  const std::string* GetVal() const { return &val; }
  std::string val;
};

// Signed integers
inline Val<CheckArgType::kInt, int> MakeVal(int x) {
  return {x};
}
inline Val<CheckArgType::kLong, long> MakeVal(long x) {
  return {x};
}
inline Val<CheckArgType::kLongLong, long long> MakeVal(long long x) {
  return {x};
}

// Unsigned integers
inline Val<CheckArgType::kUInt, unsigned int> MakeVal(unsigned int x) {
  return {x};
}
inline Val<CheckArgType::kULong, unsigned long> MakeVal(unsigned long x) {
  return {x};
}
inline Val<CheckArgType::kULongLong, unsigned long long> MakeVal(
    unsigned long long x) {
  return {x};
}

// Floating point
inline Val<CheckArgType::kDouble, double> MakeVal(double x) {
  return {x};
}
inline Val<CheckArgType::kLongDouble, long double> MakeVal(long double x) {
  return {x};
}

// Strings and pointers
inline Val<CheckArgType::kCharP, const char*> MakeVal(const char* x) {
  return {x};
}
inline Val<CheckArgType::kStdString, const std::string*> MakeVal(
    const std::string& x) {
  return {&x};
}

inline Val<CheckArgType::kStringView, const std::string_view*> MakeVal(
    const std::string_view& x) {
  return {&x};
}

inline Val<CheckArgType::kVoidP, const void*> MakeVal(const void* x) {
  return {x};
}

// The enum class types are not implicitly convertible to arithmetic types.
template <typename T,
          std::enable_if_t<std::is_enum<T>::value &&
                           !std::is_arithmetic<T>::value>* = nullptr>
inline decltype(MakeVal(std::declval<std::underlying_type_t<T>>())) MakeVal(
    T x) {
  return {static_cast<std::underlying_type_t<T>>(x)};
}

template <typename T, decltype(ToLogString(std::declval<T>()))* = nullptr>
ToStringVal MakeVal(const T& x) {
  return {ToLogString(x)};
}

// LogStreamer implementation
template <typename... Ts>
class LogStreamer;

// Base case
template <>
class LogStreamer<> final {
 public:
  template <typename T>
  LogStreamer<decltype(MakeVal(std::declval<T>()))> operator<<(
      const T& value) const {
    return LogStreamer<decltype(MakeVal(value))>(MakeVal(value), this);
  }

#if AVE_CHECK_MSG_ENABLED
  template <typename... Us>
  AVE_NORETURN void Call(const char* file,
                         int line,
                         const char* message,
                         const Us&... args) const {
    static constexpr CheckArgType t[] = {Us::Type()..., CheckArgType::kEnd};
    FatalLog(file, line, message, t, args.GetVal()...);
  }

  template <typename... Us>
  AVE_NORETURN void CallCheckOp(const char* file,
                                int line,
                                const char* message,
                                const Us&... args) const {
    static constexpr CheckArgType t[] = {CheckArgType::kCheckOp, Us::Type()...,
                                         CheckArgType::kEnd};
    FatalLog(file, line, message, t, args.GetVal()...);
  }
#else
  template <typename... Us>
  AVE_NORETURN void Call(const char* file, int line) const {
    FatalLog(file, line);
  }
#endif
};

// Inductive case: We've already seen at least one << argument.
template <typename T, typename... Ts>
class LogStreamer<T, Ts...> final {
 public:
  LogStreamer(T arg, const LogStreamer<Ts...>* prior)
      : arg_(arg), prior_(prior) {}

  template <typename U>
  LogStreamer<decltype(MakeVal(std::declval<U>())), T, Ts...> operator<<(
      const U& value) const {
    return LogStreamer<decltype(MakeVal(value)), T, Ts...>(MakeVal(value),
                                                           this);
  }

#if AVE_CHECK_MSG_ENABLED
  template <typename... Us>
  AVE_NORETURN void Call(const char* file,
                         int line,
                         const char* message,
                         const Us&... args) const {
    prior_->Call(file, line, message, arg_, args...);
  }

  template <typename... Us>
  AVE_NORETURN void CallCheckOp(const char* file,
                                int line,
                                const char* message,
                                const Us&... args) const {
    prior_->CallCheckOp(file, line, message, arg_, args...);
  }
#else
  template <typename... Us>
  AVE_NORETURN void Call(const char* file, int line) const {
    prior_->Call(file, line);
  }
#endif

 private:
  T arg_;
  const LogStreamer<Ts...>* prior_;
};

template <bool isCheckOp>
class FatalLogCall final {
 public:
  FatalLogCall(const char* file, int line, const char* message)
      : file_(file), line_(line), message_(message) {}

  template <typename... Ts>
  AVE_NORETURN void operator&(const LogStreamer<Ts...>& streamer) {
#if AVE_CHECK_MSG_ENABLED
    isCheckOp ? streamer.CallCheckOp(file_, line_, message_)
              : streamer.Call(file_, line_, message_);
#else
    streamer.Call(file_, line_);
#endif
  }

 private:
  const char* file_;
  int line_;
  const char* message_;
};

}  // namespace internal
}  // namespace ave

#if AVE_CHECK_MSG_ENABLED
#define AVE_CHECK(condition)                                                   \
  (condition)                                                                  \
      ? static_cast<void>(0)                                                   \
      : ::ave::internal::FatalLogCall<false>(__FILE__, __LINE__, #condition) & \
            ::ave::internal::LogStreamer<>()

#define AVE_CHECK_OP(name, op, val1, val2)                             \
  ::ave::base::Safe##name((val1), (val2))                              \
      ? static_cast<void>(0)                                           \
      : ::ave::internal::FatalLogCall<true>(__FILE__, __LINE__,        \
                                            #val1 " " #op " " #val2) & \
            ::ave::internal::LogStreamer<>() << (val1) << (val2)
#else
#define AVE_CHECK(condition)                                                   \
  (condition) ? static_cast<void>(0)                                           \
  : true      ? ::ave::internal::FatalLogCall<false>(__FILE__, __LINE__, "") & \
               ::ave::internal::LogStreamer<>()                                \
         : ::ave::internal::FatalLogCall<false>("", 0, "") &                   \
               ::ave::internal::LogStreamer<>()

#define AVE_CHECK_OP(name, op, val1, val2)                               \
  ::ave::base::Safe##name((val1), (val2)) ? static_cast<void>(0)         \
  : true ? ::ave::internal::FatalLogCall<true>(__FILE__, __LINE__, "") & \
               ::ave::internal::LogStreamer<>()                          \
         : ::ave::internal::FatalLogCall<false>("", 0, "") &             \
               ::ave::internal::LogStreamer<>()
#endif

#define AVE_EAT_STREAM_PARAMETERS(ignored)                \
  (true ? true : ((void)(ignored), true))                 \
      ? static_cast<void>(0)                              \
      : ::ave::internal::FatalLogCall<false>("", 0, "") & \
            ::ave::internal::LogStreamer<>()

#define AVE_EAT_STREAM_PARAMETERS_OP(op, a, b) \
  AVE_EAT_STREAM_PARAMETERS(((void)::ave::base::Safe##op(a, b)))

#define AVE_CHECK_EQ(val1, val2) AVE_CHECK_OP(Eq, ==, val1, val2)
#define AVE_CHECK_NE(val1, val2) AVE_CHECK_OP(Ne, !=, val1, val2)
#define AVE_CHECK_LE(val1, val2) AVE_CHECK_OP(Le, <=, val1, val2)
#define AVE_CHECK_LT(val1, val2) AVE_CHECK_OP(Lt, <, val1, val2)
#define AVE_CHECK_GE(val1, val2) AVE_CHECK_OP(Ge, >=, val1, val2)
#define AVE_CHECK_GT(val1, val2) AVE_CHECK_OP(Gt, >, val1, val2)

#if AVE_DCHECK_IS_ON
#define AVE_DCHECK(condition) AVE_CHECK(condition)
#define AVE_DCHECK_EQ(v1, v2) AVE_CHECK_EQ(v1, v2)
#define AVE_DCHECK_NE(v1, v2) AVE_CHECK_NE(v1, v2)
#define AVE_DCHECK_LE(v1, v2) AVE_CHECK_LE(v1, v2)
#define AVE_DCHECK_LT(v1, v2) AVE_CHECK_LT(v1, v2)
#define AVE_DCHECK_GE(v1, v2) AVE_CHECK_GE(v1, v2)
#define AVE_DCHECK_GT(v1, v2) AVE_CHECK_GT(v1, v2)
#else
#define AVE_DCHECK(condition) AVE_EAT_STREAM_PARAMETERS(condition)
#define AVE_DCHECK_EQ(v1, v2) AVE_EAT_STREAM_PARAMETERS_OP(Eq, v1, v2)
#define AVE_DCHECK_NE(v1, v2) AVE_EAT_STREAM_PARAMETERS_OP(Ne, v1, v2)
#define AVE_DCHECK_LE(v1, v2) AVE_EAT_STREAM_PARAMETERS_OP(Le, v1, v2)
#define AVE_DCHECK_LT(v1, v2) AVE_EAT_STREAM_PARAMETERS_OP(Lt, v1, v2)
#define AVE_DCHECK_GE(v1, v2) AVE_EAT_STREAM_PARAMETERS_OP(Ge, v1, v2)
#define AVE_DCHECK_GT(v1, v2) AVE_EAT_STREAM_PARAMETERS_OP(Gt, v1, v2)
#endif

#define AVE_UNREACHABLE_CODE_HIT false
#define AVE_NOTREACHED() AVE_DCHECK(AVE_UNREACHABLE_CODE_HIT)

#endif  // AVE_BASE_CHECKS_H_
