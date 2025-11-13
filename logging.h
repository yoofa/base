/*
 * logging.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_LOGGING_H
#define AVE_LOGGING_H

#include <atomic>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "base/constructor_magic.h"
#include "base/units/timestamp.h"

#if !defined(NDEBUG) || defined(DLOG_ALWAYS_ON)
#define AVE_DLOG_IS_ON 1
#else
#define AVE_DLOG_IS_ON 0
#endif

#if defined(AVE_DISABLE_LOGGING)
#define AVE_LOG_ENABLED() 0
#else
#define AVE_LOG_ENABLED() 1
#endif

namespace ave {
namespace base {

enum LogSeverity {
  LS_VERBOSE,
  LS_DEBUG,
  LS_INFO,
  LS_WARNING,
  LS_ERROR,
  LS_NONE,
};

enum LogErrorContext {
  ERRCTX_NONE,
  ERRCTX_ERRNO,

};

class LogMessage;

// LogLineRef encapsulates all the information required to generate a log line.
// It is used both internally to LogMessage but also as a parameter to
// LogSink::OnLogMessage, allowing custom LogSinks to format the log in
// the most flexible way.
class LogLineRef {
 public:
  std::string_view message() const { return message_; }
  std::string_view filename() const { return filename_; }
  int line() const { return line_; }
  std::optional<uint32_t> thread_id() const { return thread_id_; }
  Timestamp timestamp() const { return timestamp_; }
  std::string_view tag() const { return tag_; }
  LogSeverity severity() const { return severity_; }

#if AVE_LOG_ENABLED()
  std::string DefaultLogLine() const;
#else
  std::string DefaultLogLine() const { return ""; }
#endif

 private:
  friend class LogMessage;
  void set_message(std::string message) { message_ = std::move(message); }
  void set_filename(std::string_view filename) { filename_ = filename; }
  void set_line(int line) { line_ = line; }
  void set_thread_id(std::optional<uint32_t> thread_id) {
    thread_id_ = thread_id;
  }
  void set_timestamp(Timestamp timestamp) { timestamp_ = timestamp; }
  void set_tag(std::string_view tag) { tag_ = tag; }
  void set_severity(LogSeverity severity) { severity_ = severity; }

  std::string message_;
  std::string_view filename_;
  int line_ = 0;
  std::optional<uint32_t> thread_id_;
  Timestamp timestamp_ = Timestamp::MinusInfinity();
  // The default Android debug output tag.
  std::string_view tag_ = "av_engine";
  // The severity level of this message
  LogSeverity severity_;
};

class LogSink {
 public:
  LogSink() = default;
  virtual ~LogSink() = default;
  virtual void OnLogMessage(const std::string& msg,
                            LogSeverity severity,
                            const char* tag);
  virtual void OnLogMessage(const std::string& msg, LogSeverity severity);
  virtual void OnLogMessage(const std::string& msg) = 0;

  virtual void OnLogMessage(const LogLineRef& line);

 private:
  friend class ::ave::base::LogMessage;
#if AVE_LOG_ENABLED()
  LogSink* next_ = nullptr;
  LogSeverity min_severity_ = LS_VERBOSE;
#endif
};

namespace logging_impl {

class LogMetadata {
 public:
  LogMetadata(const char* file, int line, LogSeverity severity)
      : file_(file),
        line_and_sev_(static_cast<uint32_t>(static_cast<uint32_t>(line) << 3 |
                                            static_cast<uint32_t>(severity))) {}
  LogMetadata() = default;

  const char* File() const { return file_; }
  int Line() const { return static_cast<int>(line_and_sev_ >> 3); }
  LogSeverity Severity() const {
    return static_cast<LogSeverity>(line_and_sev_ & 0x7);
  }

 private:
  const char* file_;

  // Line number and severity, the former in the most significant 29 bits, the
  // latter in the least significant 3 bits. (This is an optimization; since
  // both numbers are usually compile-time constants, this way we can load them
  // both with a single instruction.)
  uint32_t line_and_sev_;
};
static_assert(std::is_trivial_v<LogMetadata>);

struct LogMetadataErr {
  LogMetadata meta;
  LogErrorContext err_ctx;
  int err;
};

#ifdef AVE_ANDROID
struct LogMetadataTag {
  LogSeverity severity;
  const char* tag;
};
#endif

enum class LogArgType : uint8_t {
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
  kLogMetadata,
  kLogMetadataErr,
#ifdef AVE_ANDROID
  kLogMetadataTag,
#endif
};

template <LogArgType N, typename T>
struct Val {
  static constexpr LogArgType Type() { return N; }
  T GetVal() const { return val; }
  T val;
};

struct ToStringVal {
  static constexpr LogArgType Type() { return LogArgType::kStdString; }
  const std::string* GetVal() const { return &val; }
  std::string val;
};

inline Val<LogArgType::kInt, int> MakeVal(int x) {
  return {x};
}

inline Val<LogArgType::kLong, long> MakeVal(long x) {
  return {x};
}

inline Val<LogArgType::kLongLong, long long> MakeVal(long long x) {
  return {x};
}

inline Val<LogArgType::kUInt, unsigned int> MakeVal(unsigned int x) {
  return {x};
}

inline Val<LogArgType::kULong, unsigned long> MakeVal(unsigned long x) {
  return {x};
}

inline Val<LogArgType::kULongLong, unsigned long long> MakeVal(
    unsigned long long x) {
  return {x};
}

inline Val<LogArgType::kDouble, double> MakeVal(double x) {
  return {x};
}

inline Val<LogArgType::kLongDouble, long double> MakeVal(long double x) {
  return {x};
}

inline Val<LogArgType::kCharP, const char*> MakeVal(const char* x) {
  return {x};
}

inline Val<LogArgType::kStdString, const std::string*> MakeVal(
    const std::string& x) {
  return {&x};
}

inline Val<LogArgType::kStringView, const std::string_view*> MakeVal(
    const std::string_view& x) {
  return {&x};
}

inline Val<LogArgType::kVoidP, const void*> MakeVal(const void* x) {
  return {x};
}

inline Val<LogArgType::kLogMetadata, LogMetadata> MakeVal(
    const LogMetadata& x) {
  return {x};
}

inline Val<LogArgType::kLogMetadataErr, LogMetadataErr> MakeVal(
    const LogMetadataErr& x) {
  return {x};
}

// The enum class types are not implicitly convertible to arithmetic types.
template <
    typename T,
    std::enable_if_t<std::is_enum_v<T> && !std::is_arithmetic_v<T>>* = nullptr>
inline decltype(MakeVal(std::declval<std::underlying_type_t<T>>())) MakeVal(
    T x) {
  return {static_cast<std::underlying_type_t<T>>(x)};
}

#ifdef AVE_ANDROID
inline Val<LogArgType::kLogMetadataTag, LogMetadataTag> MakeVal(
    const LogMetadataTag& x) {
  return {x};
}
#endif

template <typename T, class = std::void_t<>>
struct has_to_log_string : std::false_type {};
template <typename T>
struct has_to_log_string<T,
                         std::void_t<decltype(ToLogString(std::declval<T>()))>>
    : std::true_type {};

template <
    typename T,
    typename T1 = std::decay_t<T>,
    typename = std::enable_if_t<
        std::is_class_v<T1> && !std::is_same_v<T1, std::string> &&
#ifdef AVE_ANDROID
        !std::is_same_v<T1, LogMetadataTag> &&
#endif
        !std::is_same_v<T1, LogMetadata> && !has_to_log_string<T1>::value>>
ToStringVal MakeVal(const T& x) {
  std::ostringstream os;
  os << x;
  return {os.str()};
}

template <typename T, typename = std::enable_if_t<has_to_log_string<T>::value>>
ToStringVal MakeVal(const T& x) {
  return {ToLogString(x)};
}

#if AVE_LOG_ENABLED()
void Log(const LogArgType* fmt, ...);
#else
inline void Log(const LogArgType* fmt, ...) {
  // Do nothing, shouldn't be invoked
}
#endif

template <typename... Ts>
class LogStreamer;

template <>
class LogStreamer<> final {
 public:
  template <
      typename U,
      typename V = decltype(MakeVal(std::declval<U>())),
      std::enable_if_t<std::is_arithmetic_v<U> || std::is_enum_v<U>>* = nullptr>
  inline LogStreamer<V> operator<<(U arg) const {
    return LogStreamer<V>(MakeVal(arg), this);
  }

  template <typename U,
            typename V = decltype(MakeVal(std::declval<U>())),
            std::enable_if_t<!std::is_arithmetic_v<U> && !std::is_enum_v<U>>* =
                nullptr>
  inline LogStreamer<V> operator<<(const U& arg) const {
    return LogStreamer<V>(MakeVal(arg), this);
  }

  template <typename... Us>
  inline static void Call(const Us&... args) {
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    static constexpr LogArgType t[] = {Us::Type()..., LogArgType::kEnd};
    // NOLINTEND(modernize-avoid-c-arrays)
    Log(t, args.GetVal()...);
  }
};

template <typename T, typename... Ts>
class LogStreamer<T, Ts...> final {
 public:
  inline LogStreamer(T arg, const LogStreamer<Ts...>* prior)
      : arg_(arg), prior_(prior) {}

  template <
      typename U,
      typename V = decltype(MakeVal(std::declval<U>())),
      std::enable_if_t<std::is_arithmetic_v<U> || std::is_enum_v<U>>* = nullptr>
  inline LogStreamer<V, T, Ts...> operator<<(U arg) const {
    return LogStreamer<V, T, Ts...>(MakeVal(arg), this);
  }

  template <typename U,
            typename V = decltype(MakeVal(std::declval<U>())),
            std::enable_if_t<!std::is_arithmetic_v<U> && !std::is_enum_v<U>>* =
                nullptr>
  inline LogStreamer<V, T, Ts...> operator<<(const U& arg) const {
    return LogStreamer<V, T, Ts...>(MakeVal(arg), this);
  }

  template <typename... Us>
  inline void Call(const Us&... args) const {
    prior_->Call(arg_, args...);
  }

 private:
  T arg_;

  const LogStreamer<Ts...>* prior_;
};

class Logger final {
 public:
  template <typename... Ts>
  inline bool operator&(const LogStreamer<Ts...>& streamer) {
    streamer.Call();
    return true;
  }
};

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify {
 public:
  LogMessageVoidify() = default;
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  template <typename... Ts>
  void operator&(LogStreamer<Ts...>&& streamer) {}
};

} /* namespace logging_impl */

class LogMessage {
 public:
  template <LogSeverity S>
  inline LogMessage(const char* file,
                    int line,
                    std::integral_constant<LogSeverity, S>)
      : LogMessage(file, line, S) {}

#if AVE_LOG_ENABLED()
  LogMessage(const char* file, int line, LogSeverity sev);
  LogMessage(const char* file,
             int line,
             LogSeverity sev,
             LogErrorContext err_ctx,
             int err);
#if defined(AVE_ANDROID)
  LogMessage(const char* file, int line, LogSeverity sev, const char* tag);
#endif

  ~LogMessage();

  void AddTag(const char* tag);
  std::stringstream& stream();
  static int64_t LogStartTime();
  static uint32_t WallClockStartTime();
  static void LogThreads(bool on = true);
  static void LogPrintSeverity(bool on = true);
  static void LogTimestamps(bool on = true);
  static void LogToDebug(LogSeverity min_sev);
  static LogSeverity GetLogToDebug();
  static void SetLogToStderr(bool log_to_stderr);
  static void AddLogToStream(LogSink* stream, LogSeverity min_sev);
  static void RemoveLogToStream(LogSink* stream);
  static int GetLogToStream(LogSink* stream = nullptr);
  static int GetMinLogSeverity();
  static bool IsNoop(LogSeverity severity);
  template <LogSeverity S>
  inline static bool IsNoop() {
    return IsNoop(S);
  }
#else
  LogMessage(const char* file, int line, LogSeverity sev) {}
  LogMessage(const char* file,
             int line,
             LogSeverity sev,
             LogErrorContext err_ctx,
             int err) {}
#if defined(AVE_ANDROID)
  LogMessage(const char* file, int line, LogSeverity sev, const char* tag) {}
#endif
  ~LogMessage() = default;

  inline void AddTag(const char* tag) {}
  inline std::stringstream& stream() { return print_stream_; }
  inline static int64_t LogStartTime() { return 0; }
  inline static uint32_t WallClockStartTime() { return 0; }
  inline static void LogThreads(bool on = true) {}
  inline static void LogPrintSeverity(bool on = true) {}
  inline static void LogTimestamps(bool on = true) {}
  inline static void LogToDebug(LogSeverity min_sev) {}
  inline static LogSeverity GetLogToDebug() { return LS_INFO; }
  inline static void SetLogToStderr(bool log_to_stderr) {}
  inline static void AddLogToStream(LogSink* stream, LogSeverity min_sev) {}
  inline static void RemoveLogToStream(LogSink* stream) {}
  inline static int GetLogToStream(LogSink* stream = nullptr) { return 0; }
  inline static int GetMinLogSeverity() { return 0; }
  static constexpr bool IsNoop(LogSeverity severity) { return true; }
  template <LogSeverity S>
  static constexpr bool IsNoop() {
    return IsNoop(S);
  }
#endif

 private:
#if AVE_LOG_ENABLED()
  static void UpdateMinLogSeverity();

  // static void OutputToDebug(const std::string& msg, LogSeverity severity);
  static void OutputToDebug(const LogLineRef& log_line_ref);

  void FinishPrintStream();

  LogLineRef log_line_;

  std::string extra_;

  static LogSink* streams_;

  static std::atomic<bool> streams_empty_;

  static bool thread_, timestamp_, print_severity_;

  static bool log_to_stderr_;
#else
  inline static void UpdateMinLogSeverity() {}
#ifdef AVE_ANDROID
  inline static void OutputToDebug(const std::string& msg,
                                   LogSeverity severity) {}
#else
  inline static void OutputToDebug(const std::string& msg,
                                   LogSeverity severity) {}
#endif
  inline void FinishPrintStream() {}
#endif

  std::stringstream print_stream_;

  AVE_DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

#define AVE_LOG_FILE_LINE(sev, file, line)       \
  ::ave::base::logging_impl::Logger() &          \
      ::ave::base::logging_impl::LogStreamer<>() \
          << ::ave::base::logging_impl::LogMetadata(file, line, sev)

#define AVE_LOG(sev)                       \
  !ave::base::LogMessage::IsNoop<sev>() && \
      AVE_LOG_FILE_LINE(sev, __FILE__, __LINE__)

#if AVE_DLOG_IS_ON
#define AVE_DLOG(sev) AVE_LOG(sev)
#define AVE_DLOG_IF(sev, condition) AVE_LOG_IF(sev, condition)
#define AVE_DLOG_V(sev) AVE_LOG_V(sev)
#define AVE_DLOG_F(sev) AVE_LOG_F(sev)
#define AVE_DLOG_IF_F(sev, condition) AVE_LOG_IF_F(sev, condition)
#else
#define AVE_DLOG_EAT_STREAM_PARAMS()               \
  while (false)                                    \
  ::ave::base::logging_impl::LogMessageVoidify() & \
      (::ave::base::logging_impl::LogStreamer<>())
#define AVE_DLOG(sev) AVE_DLOG_EAT_STREAM_PARAMS()
#define AVE_DLOG_IF(sev, condition) AVE_DLOG_EAT_STREAM_PARAMS()
#define AVE_DLOG_V(sev) AVE_DLOG_EAT_STREAM_PARAMS()
#define AVE_DLOG_F(sev) AVE_DLOG_EAT_STREAM_PARAMS()
#define AVE_DLOG_IF_F(sev, condition) AVE_DLOG_EAT_STREAM_PARAMS()
#endif

}  // namespace base
// LS_VERBOSE, LS_DEBUG, LS_INFO, LS_WARNING, LS_ERROR

using base::LS_DEBUG;
using base::LS_ERROR;
using base::LS_INFO;
using base::LS_VERBOSE;
using base::LS_WARNING;

}  // namespace ave
using ave::base::LS_DEBUG;
using ave::base::LS_ERROR;
using ave::base::LS_INFO;
using ave::base::LS_VERBOSE;
using ave::base::LS_WARNING;

#endif /* !LOGGING_H */
