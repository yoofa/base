/*
 * logging.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/logging.h"

#include <cstring>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <string>
#include <thread>

#include "base/time_utils.h"

#if defined(AVE_ANDROID)
#include <android/log.h>
// Android has a 1024 limit on log inputs. We use 60 chars as an
// approx for the header/tag portion.
// See android/system/core/liblog/logd_write.c
static const int kMaxLogLineSize = 1024 - 60;
#endif

#include "base/attributes.h"

namespace ave {
namespace base {

namespace {
#if !defined(NDEBUG)
LogSeverity g_min_sev = LS_INFO;
LogSeverity g_dbg_sev = LS_INFO;
#else
LogSeverity g_min_sev = LS_NONE;
LogSeverity g_dbg_sev = LS_NONE;
#endif

const char* FilenameFromPath(const char* file) {
  const char* end1 = ::strrchr(file, '/');
  const char* end2 = ::strrchr(file, '\\');
  if (!end1 && !end2) {
    return file;
  }
  return (end1 > end2) ? end1 + 1 : end2 + 1;
}

std::mutex g_log_mutex_;

}  // namespace

std::string formatTimeMillis(Timestamp timestamp) {
  time_t seconds = timestamp.ms_or(0) / 1000;
  auto milliseconds = timestamp.ms_or(0) % 1000;

  std::tm tmStruct{};
  localtime_r(&seconds, &tmStruct);

  std::ostringstream oss;
  oss << std::put_time(&tmStruct, "%m-%d %H:%M:%S") << '.' << std::setfill('0')
      << std::setw(3) << milliseconds;

  return oss.str();
}

std::string LogLineRef::DefaultLogLine() const {
  std::stringstream log_output;
  if (timestamp_ != Timestamp::MinusInfinity()) {
    log_output << formatTimeMillis(timestamp_) << " ";
  }

  if (thread_id_.has_value()) {
    log_output << "[" << *thread_id_ << "] ";
  }

  if (!filename_.empty()) {
    log_output << "(" << filename_ << ":" << line_ << "): ";
  }

  log_output << message_;
  return log_output.str();
}

// static member
bool LogMessage::log_to_stderr_ = true;

LogSink* LogMessage::streams_ = nullptr;
std::atomic<bool> LogMessage::streams_empty_ = {true};

#if defined(AVE_ANDROID)
bool LogMessage::thread_ = false;
bool LogMessage::timestamp_ = false;
#else
bool LogMessage::thread_ = true;
bool LogMessage::timestamp_ = true;

#endif

LogMessage::LogMessage(const char* file, int line, LogSeverity sev)
    : LogMessage(file, line, sev, ERRCTX_NONE, 0) {}

LogMessage::LogMessage(const char* file,
                       int line,
                       LogSeverity sev,
                       LogErrorContext err_ctx,
                       int err) {
  log_line_.set_severity(sev);
  if (timestamp_) {
    auto log_start_time = LogStartTime();
    // Use SystemTimeMillis so that even if tests use fake clocks, the timestamp
    // in log messages represents the real system time.
    auto time = TimeDiff(SystemTimeMillis(), log_start_time);
    // Also ensure WallClockStartTime is initialized, so that it matches
    // LogStartTime.
    WallClockStartTime();
    log_line_.set_timestamp(Timestamp::Millis(time));
  }

  if (thread_) {
    log_line_.set_thread_id(
        std::hash<std::thread::id>{}(std::this_thread::get_id()));
  }

  if (file != nullptr) {
    log_line_.set_filename(FilenameFromPath(file));
    log_line_.set_line(line);
  }

  if (err_ctx != ERRCTX_NONE) {
    std::stringstream tmp;
    switch (err_ctx) {
      case ERRCTX_ERRNO:
        tmp << " " << strerror(err);
        break;
      default:
        break;
    }
    extra_ = tmp.str();
  }
}

#if defined(AVE_ANDROID)
LogMessage::LogMessage(const char* file,
                       int line,
                       LogSeverity sev,
                       const char* tag)
    : LogMessage(file, line, sev, ERRCTX_NONE, /*err=*/0) {
  log_line_.set_tag(tag);
  LogThreads(false);
  LogTimestamps(false);
  print_stream_ << tag << ": ";
}
#endif

LogMessage::~LogMessage() {
  FinishPrintStream();
  log_line_.set_message(print_stream_.str());
  if (log_line_.severity() >= g_dbg_sev) {
    OutputToDebug(log_line_);
  }

  std::lock_guard<std::mutex> guard(g_log_mutex_);
  for (LogSink* entry = streams_; entry != nullptr; entry = entry->next_) {
    if (log_line_.severity() >= entry->min_severity_) {
      entry->OnLogMessage(log_line_);
    }
  }
}

void LogMessage::AddTag(const char* tag) {
#ifdef AVE_ANDROID
  log_line_.set_tag(tag);
#endif
}

std::stringstream& LogMessage::stream() {
  return print_stream_;
}

int LogMessage::GetMinLogSeverity() {
  return static_cast<int>(g_min_sev);
}

LogSeverity LogMessage::GetLogToDebug() {
  return g_dbg_sev;
}

int64_t LogMessage::LogStartTime() {
  static const int64_t g_start = SystemTimeMillis();
  return g_start;
}

uint32_t LogMessage::WallClockStartTime() {
  static const uint32_t g_start_wallclock = time(nullptr);
  return g_start_wallclock;
}

void LogMessage::LogThreads(bool on) {
  thread_ = on;
}

void LogMessage::LogTimestamps(bool on) {
  timestamp_ = on;
}

void LogMessage::LogToDebug(LogSeverity min_sev) {
  g_dbg_sev = min_sev;
  std::lock_guard<std::mutex> guard(g_log_mutex_);
  UpdateMinLogSeverity();
}

void LogMessage::SetLogToStderr(bool log_to_stderr) {
  log_to_stderr_ = log_to_stderr;
}

int LogMessage::GetLogToStream(LogSink* stream) {
  std::lock_guard<std::mutex> guard(g_log_mutex_);
  LogSeverity sev = LS_NONE;
  for (LogSink* entry = streams_; entry != nullptr; entry = entry->next_) {
    if (stream == nullptr || stream == entry) {
      sev = std::min(sev, entry->min_severity_);
    }
  }
  return static_cast<int>(sev);
}

void LogMessage::AddLogToStream(LogSink* stream, LogSeverity min_sev) {
  std::lock_guard<std::mutex> guard(g_log_mutex_);
  stream->min_severity_ = min_sev;
  stream->next_ = streams_;
  streams_ = stream;
  streams_empty_.store(false, std::memory_order_relaxed);
  UpdateMinLogSeverity();
}

void LogMessage::RemoveLogToStream(LogSink* stream) {
  std::lock_guard<std::mutex> guard(g_log_mutex_);
  for (LogSink** entry = &streams_; *entry != nullptr;
       entry = &(*entry)->next_) {
    if (*entry == stream) {
      *entry = (*entry)->next_;
      break;
    }
  }
  streams_empty_.store(streams_ == nullptr, std::memory_order_relaxed);
  UpdateMinLogSeverity();
}

void LogMessage::UpdateMinLogSeverity() {
  LogSeverity min_sev = g_dbg_sev;
  for (LogSink* entry = streams_; entry != nullptr; entry = entry->next_) {
    min_sev = std::min(min_sev, entry->min_severity_);
  }
  g_min_sev = min_sev;
}

void LogMessage::OutputToDebug(const LogLineRef& log_line) {
  std::string msg_str = log_line.DefaultLogLine();
  bool log_to_stderr = log_to_stderr_;
#if defined(AVE_MAC) && !defined(AVE_IOS) && defined(NDEBUG)
  // On the Mac, all stderr output goes to the Console log and causes clutter.
  // So in opt builds, don't log to stderr unless the user specifically sets
  // a preference to do so.
  CFStringRef domain = CFBundleGetIdentifier(CFBundleGetMainBundle());
  if (domain != nullptr) {
    Boolean exists_and_is_valid;
    Boolean should_log = CFPreferencesGetAppBooleanValue(
        CFSTR("logToStdErr"), domain, &exists_and_is_valid);
    // If the key doesn't exist or is invalid or is false, we will not log to
    // stderr.
    log_to_stderr = exists_and_is_valid && should_log;
  }
#endif  // defined(AVE_MAC) && !defined(AVE_IOS) && defined(NDEBUG)
#if defined(AVE_WIN)
  // Always log to the debugger.
  // Perhaps stderr should be controlled by a preference, as on Mac?
  OutputDebugStringA(msg_str.c_str());
  if (log_to_stderr) {
    // This handles dynamically allocated consoles, too.
    if (HANDLE error_handle = ::GetStdHandle(STD_ERROR_HANDLE)) {
      log_to_stderr = false;
      DWORD written = 0;
      ::WriteFile(error_handle, msg_str.c_str(),
                  static_cast<DWORD>(msg_str.size()), &written, 0);
    }
  }
#endif  // AVE_WIN
#if defined(AVE_ANDROID)
  // Android's logging facility uses severity to log messages but we
  // need to map libjingle's severity levels to Android ones first.
  // Also write to stderr which maybe available to executable started
  // from the shell.
  int prio = 0;
  switch (log_line.severity()) {
    case LS_VERBOSE:
      prio = ANDROID_LOG_VERBOSE;
      break;
    case LS_DEBUG:
      prio = ANDROID_LOG_DEBUG;
      break;
    case LS_INFO:
      prio = ANDROID_LOG_INFO;
      break;
    case LS_WARNING:
      prio = ANDROID_LOG_WARN;
      break;
    case LS_ERROR:
      prio = ANDROID_LOG_ERROR;
      break;
    default:
      prio = ANDROID_LOG_UNKNOWN;
  }
  int size = static_cast<int>(msg_str.size());
  int current_line = 0;
  int idx = 0;
  const int max_lines = size / kMaxLogLineSize + 1;
  if (max_lines == 1) {
    __android_log_print(prio, log_line.tag().data(), "%.*s", size,
                        msg_str.c_str());
  } else {
    while (size > 0) {
      const int len = std::min(size, kMaxLogLineSize);
      // Use the size of the string in the format (msg may have \0 in the
      // middle).
      __android_log_print(prio, log_line.tag().data(), "[%d/%d] %.*s",
                          current_line + 1, max_lines, len,
                          msg_str.c_str() + idx);
      idx += len;
      size -= len;
      ++current_line;
    }
  }
#endif  // AVE_ANDROID
  if (log_to_stderr) {
    fprintf(stderr, "%s", msg_str.c_str());
    fflush(stderr);
  }
}

// static
bool LogMessage::IsNoop(LogSeverity severity) {
  if (severity >= g_dbg_sev || severity >= g_min_sev) {
    return false;
  }
  return streams_empty_.load(std::memory_order_relaxed);
}

void LogMessage::FinishPrintStream() {
  if (!extra_.empty()) {
    print_stream_ << " : " << extra_;
  }
  print_stream_ << "\n";
}

namespace logging_impl {

void Log(const LogArgType* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  LogMetadataErr meta{};
  const char* tag = nullptr;
  switch (*fmt) {
    case LogArgType::kLogMetadata: {
      meta = {va_arg(args, LogMetadata), ERRCTX_NONE, 0};
      break;
    }
    case LogArgType::kLogMetadataErr: {
      meta = va_arg(args, LogMetadataErr);
      break;
    }
#ifdef AVE_ANDROID
    case LogArgType::kLogMetadataTag: {
      const LogMetadataTag tag_meta = va_arg(args, LogMetadataTag);
      meta = {{nullptr, 0, tag_meta.severity}, ERRCTX_NONE, 0};
      tag = tag_meta.tag;
      break;
    }
#endif
    default: {
      va_end(args);
      return;
    }
  }

  LogMessage log_message(meta.meta.File(), meta.meta.Line(),
                         meta.meta.Severity(), meta.err_ctx, meta.err);
  if (tag) {
    log_message.AddTag(tag);
  }

  for (++fmt; *fmt != LogArgType::kEnd; ++fmt) {
    switch (*fmt) {
      case LogArgType::kInt:
        log_message.stream() << va_arg(args, int);
        break;
      case LogArgType::kLong:
        log_message.stream() << va_arg(args, long);
        break;
      case LogArgType::kLongLong:
        log_message.stream() << va_arg(args, long long);
        break;
      case LogArgType::kUInt:
        log_message.stream() << va_arg(args, unsigned);
        break;
      case LogArgType::kULong:
        log_message.stream() << va_arg(args, unsigned long);
        break;
      case LogArgType::kULongLong:
        log_message.stream() << va_arg(args, unsigned long long);
        break;
      case LogArgType::kDouble:
        log_message.stream() << va_arg(args, double);
        break;
      case LogArgType::kLongDouble:
        log_message.stream() << va_arg(args, long double);
        break;
      case LogArgType::kCharP: {
        const char* s = va_arg(args, const char*);
        log_message.stream() << (s ? s : "(null)");
        break;
      }
      case LogArgType::kStdString:
        log_message.stream() << *va_arg(args, const std::string*);
        break;
      case LogArgType::kVoidP:
        log_message.stream()
            << std::hex
            << reinterpret_cast<uintptr_t>(va_arg(args, const void*));
        break;
      default:
        va_end(args);
        return;
    }
  }
  va_end(args);
}
}  // namespace logging_impl

// Default implementation, override is recomended.
void LogSink::OnLogMessage(const LogLineRef& log_line) {
#if defined(AVE_ANDROID)
  OnLogMessage(log_line.DefaultLogLine(), log_line.severity(),
               log_line.tag().data());
#else
  OnLogMessage(log_line.DefaultLogLine(), log_line.severity());
#endif
}

void LogSink::OnLogMessage(const std::string& msg,
                           LogSeverity severity,
                           const char* tag) {
  OnLogMessage(tag + (": " + msg), severity);
}

void LogSink::OnLogMessage(const std::string& msg, LogSeverity /* severity */) {
  OnLogMessage(msg);
}

}  // namespace base
}  // namespace ave
