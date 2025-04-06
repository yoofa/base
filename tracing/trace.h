/*
 * trace.h
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#pragma once
#ifndef AVE_BASE_TRACE_H_H_
#define AVE_BASE_TRACE_H_H_

#include <cstdint>  // For uint64_t
#include <memory>   // For std::shared_ptr
#include <string>
#include <string_view>  // For std::string_view

namespace ave {
namespace tracing {

enum class TraceBackendType {
  TRACE_TYPE_NONE,
  TRACE_TYPE_PERFETTO_SYSTEM,
  TRACE_TYPE_PERFETTO_IN_PROCESS,
  TRACE_TYPE_SYSTRACE,
  TRACE_TYPE_JSON_FILE
  // ... other types
};

// Configuration structure for trace initialization
struct TraceConfig {
  TraceBackendType type = TraceBackendType::TRACE_TYPE_NONE;
  // --- Perfetto In-Process specific ---
  std::string perfetto_output_path;
  size_t perfetto_buffer_kb = 1024;
  // --- JSON File specific ---
  std::string json_output_path;
  // --- Common ---
  // std::vector<std::string> enabled_categories; // Maybe?
};

// --- Forward Declaration ---
class AbstractTracer;  // Implementation interface

// --- Public Facing API ---
class Trace {
 public:
  // --- Initialization & Control ---
  /**
   * @brief Initialize the tracing system with the specified configuration.
   * @param config Configuration for the trace system.
   * @return true if initialization was successful, false otherwise.
   */
  static bool initialize(const TraceConfig& config);

  /**
   * @brief Initialize the tracing system with a specific tracer implementation.
   * @param tracerImpl A pointer to an AbstractTracer implementation.
   * If nullptr, tracing will be disabled.
   * The caller is responsible for the lifecycle of tracerImpl.
   */
  static void initialize(std::shared_ptr<AbstractTracer> tracer_impl);

  /**
   * @brief Shut down the tracing system and release resources.
   */
  static void shutdown();

  /**
   * @brief Check if tracing is enabled.
   * @return true if tracing is enabled, false otherwise.
   */
  static bool isEnabled();

  /**
   * @brief Check if tracing for a specific category is enabled.
   * @param category The trace category to check.
   * @return true if tracing for the category is enabled, false otherwise.
   */
  static bool isCategoryEnabled(std::string_view category);

  // --- Tracing Operations ---

  /**
   * @brief Begin a trace section (scope).
   * It's recommended to use the TRACE_SCOPE macro instead of calling this
   * directly.
   * @param category The trace category.
   * @param name The event name.
   */
  static void beginSection(std::string_view category, std::string_view name);

  /**
   * @brief End the most recently started trace section.
   * It's recommended to use the TRACE_SCOPE macro instead of calling this
   * directly.
   */
  static void endSection();

  /**
   * @brief Record an instant event.
   * @param category The trace category.
   * @param name The event name.
   */
  static void instantEvent(std::string_view category, std::string_view name);

  /**
   * @brief Set the current value of a counter.
   * @param category The trace category.
   * @param name The counter name.
   * @param value The counter value.
   */
  static void setCounter(std::string_view category,
                         std::string_view name,
                         int64_t value);

  /**
   * @brief Set the current value of a counter (floating point version).
   * @param category The trace category.
   * @param name The counter name.
   * @param value The counter value.
   */
  static void setCounter(std::string_view category,
                         std::string_view name,
                         double value);

  /**
   * @brief Set the current value of a counter (integer version).
   * @param category The trace category.
   * @param name The counter name.
   * @param value The counter value.
   */
  static void setCounter(std::string_view category,
                         std::string_view name,
                         int value);

  /**
   * @brief Begin an asynchronous event.
   * @param category The trace category.
   * @param name The async event name.
   * @param cookie A unique identifier to associate with subsequent asyncEnd or
   * asyncStep calls. Typically a pointer or hash value can be used as a cookie.
   */
  static void beginAsyncEvent(std::string_view category,
                              std::string_view name,
                              uint64_t cookie);

  /**
   * @brief End an asynchronous event.
   * @param category The trace category.
   * @param name The async event name (should match beginAsyncEvent).
   * @param cookie The unique identifier (should match beginAsyncEvent).
   */
  static void endAsyncEvent(std::string_view category,
                            std::string_view name,
                            uint64_t cookie);

  /**
   * @brief Add a step to an ongoing asynchronous event.
   * @param category The trace category.
   * @param name The async event name.
   * @param cookie The unique identifier (should match beginAsyncEvent).
   * @param step_name The name of this step in the async operation.
   */
  static void asyncStepEvent(std::string_view category,
                             std::string_view name,
                             uint64_t cookie,
                             std::string_view step_name);

 private:
  static std::shared_ptr<AbstractTracer> s_tracer;
  static bool s_enabled;  // Fast check if enabled
};

// --- RAII Helper for Scopes ---
class ScopedTrace {
 public:
  // Prevent copying and moving to ensure RAII correctness
  ScopedTrace(const ScopedTrace&) = delete;
  ScopedTrace& operator=(const ScopedTrace&) = delete;
  ScopedTrace(ScopedTrace&&) = delete;
  ScopedTrace& operator=(ScopedTrace&&) = delete;

  /**
   * @brief Constructor: Starts a trace section if tracing is enabled for the
   * category.
   * @param category The trace category.
   * @param name The section name.
   */
  ScopedTrace(std::string_view category, std::string_view name)
      : active_(Trace::isCategoryEnabled(
            category))  // Check if tracing needed at construction
  {
    if (active_) {
      Trace::beginSection(category, name);
    }
  }

  /**
   * @brief Destructor: Ends the trace section if one was started.
   */
  ~ScopedTrace() {
    if (active_) {
      Trace::endSection();
    }
  }

 private:
  bool active_;  // Flag indicating if this scope actually started tracing
};

// --- Convenience Macros ---

// Default category, can be defined globally or passed at compile time
#ifndef TRACE_DEFAULT_CATEGORY
#define TRACE_DEFAULT_CATEGORY "default"
#endif

// Compile-time disable macros: If AVE_ENABLE_TRACING is not defined, all trace
// macros become no-ops
#ifndef AVE_ENABLE_TRACING

#define TRACE_INITIALIZE(config) ((void)0)
#define TRACE_SHUTDOWN() ((void)0)
#define TRACE_ENABLED() (false)
#define TRACE_CATEGORY_ENABLED(category) (false)
#define TRACE_SCOPE(name) ((void)0)
#define TRACE_SCOPE_CATEGORY(category, name) ((void)0)
#define TRACE_EVENT(name) ((void)0)
#define TRACE_EVENT_CATEGORY(category, name) ((void)0)
#define TRACE_COUNTER(name, value) ((void)0)
#define TRACE_COUNTER_CATEGORY(category, name, value) ((void)0)
#define TRACE_ASYNC_BEGIN(name, cookie) ((void)0)
#define TRACE_ASYNC_BEGIN_CATEGORY(category, name, cookie) ((void)0)
#define TRACE_ASYNC_END(name, cookie) ((void)0)
#define TRACE_ASYNC_END_CATEGORY(category, name, cookie) ((void)0)
#define TRACE_ASYNC_STEP(name, cookie, step) ((void)0)
#define TRACE_ASYNC_STEP_CATEGORY(category, name, cookie, step) ((void)0)

#else  // AVE_ENABLE_TRACING is defined

#define TRACE_INITIALIZE(config) ::ave::tracing::Trace::initialize(config)
#define TRACE_SHUTDOWN() ::ave::tracing::Trace::shutdown()
#define TRACE_ENABLED() ::ave::tracing::Trace::isEnabled()
#define TRACE_CATEGORY_ENABLED(category) \
  ::ave::tracing::Trace::isCategoryEnabled(category)

// Use macros to create ScopedTrace objects, leveraging C++ RAII to
// automatically end sections
#define TRACE_SCOPE(name)                                  \
  ::ave::tracing::ScopedTrace AVE_TRACE_UID(trace_scope_)( \
      TRACE_DEFAULT_CATEGORY, name)

#define TRACE_SCOPE_CATEGORY(category, name) \
  ::ave::tracing::ScopedTrace AVE_TRACE_UID(trace_scope_)(category, name)

#define TRACE_EVENT(name)                                                 \
  if (::ave::tracing::Trace::isCategoryEnabled(TRACE_DEFAULT_CATEGORY)) { \
    ::ave::tracing::Trace::instantEvent(TRACE_DEFAULT_CATEGORY, name);    \
  }
#define TRACE_EVENT_CATEGORY(category, name)                \
  if (::ave::tracing::Trace::isCategoryEnabled(category)) { \
    ::ave::tracing::Trace::instantEvent(category, name);    \
  }

#define TRACE_COUNTER(name, value)                                          \
  if (::ave::tracing::Trace::isCategoryEnabled(TRACE_DEFAULT_CATEGORY)) {   \
    ::ave::tracing::Trace::setCounter(TRACE_DEFAULT_CATEGORY, name, value); \
  }
#define TRACE_COUNTER_CATEGORY(category, name, value)         \
  if (::ave::tracing::Trace::isCategoryEnabled(category)) {   \
    ::ave::tracing::Trace::setCounter(category, name, value); \
  }

#define TRACE_ASYNC_BEGIN(name, cookie)                                   \
  if (::ave::tracing::Trace::isCategoryEnabled(TRACE_DEFAULT_CATEGORY)) { \
    ::ave::tracing::Trace::beginAsyncEvent(TRACE_DEFAULT_CATEGORY, name,  \
                                           cookie);                       \
  }
#define TRACE_ASYNC_BEGIN_CATEGORY(category, name, cookie)          \
  if (::ave::tracing::Trace::isCategoryEnabled(category)) {         \
    ::ave::tracing::Trace::beginAsyncEvent(category, name, cookie); \
  }

#define TRACE_ASYNC_END(name, cookie)                                     \
  if (::ave::tracing::Trace::isCategoryEnabled(TRACE_DEFAULT_CATEGORY)) { \
    ::ave::tracing::Trace::endAsyncEvent(TRACE_DEFAULT_CATEGORY, name,    \
                                         cookie);                         \
  }
#define TRACE_ASYNC_END_CATEGORY(category, name, cookie)          \
  if (::ave::tracing::Trace::isCategoryEnabled(category)) {       \
    ::ave::tracing::Trace::endAsyncEvent(category, name, cookie); \
  }

#define TRACE_ASYNC_STEP(name, cookie, step)                              \
  if (::ave::tracing::Trace::isCategoryEnabled(TRACE_DEFAULT_CATEGORY)) { \
    ::ave::tracing::Trace::asyncStepEvent(TRACE_DEFAULT_CATEGORY, name,   \
                                          cookie, step);                  \
  }
#define TRACE_ASYNC_STEP_CATEGORY(category, name, cookie, step)          \
  if (::ave::tracing::Trace::isCategoryEnabled(category)) {              \
    ::ave::tracing::Trace::asyncStepEvent(category, name, cookie, step); \
  }

#endif  // AVE_ENABLE_TRACING

// --- Implementation Interface (to be implemented by Perfetto/Systrace/etc.)
// ---
class AbstractTracer {
 public:
  virtual ~AbstractTracer() = default;

  /**
   * @brief Optional initialization method.
   * @return true if initialization was successful, false otherwise.
   */
  virtual bool initialize() { return true; }

  /**
   * @brief Optional cleanup method.
   */
  virtual void shutdown() {}

  /**
   * @brief Check if this implementation considers tracing to be enabled.
   * @return true if tracing is enabled (e.g., successfully connected to
   * service).
   */
  virtual bool isEnabled() = 0;

  /**
   * @brief Check if this implementation has enabled a specific category.
   * @param category The category to check.
   * @return true if the category is enabled.
   */
  virtual bool isCategoryEnabled(std::string_view category) = 0;

  virtual void beginSection(std::string_view category,
                            std::string_view name) = 0;
  virtual void endSection() = 0;
  virtual void instantEvent(std::string_view category,
                            std::string_view name) = 0;
  virtual void setCounter(std::string_view category,
                          std::string_view name,
                          int64_t value) = 0;
  virtual void setCounter(std::string_view category,
                          std::string_view name,
                          double value) = 0;
  virtual void beginAsyncEvent(std::string_view category,
                               std::string_view name,
                               uint64_t cookie) = 0;
  virtual void endAsyncEvent(std::string_view category,
                             std::string_view name,
                             uint64_t cookie) = 0;
  virtual void asyncStepEvent(std::string_view category,
                              std::string_view name,
                              uint64_t cookie,
                              std::string_view step_name) = 0;
};

#define AVE_TRACE_UID(prefix) AVE_TRACE_UID2(prefix, __COUNTER__)
#define AVE_TRACE_UID2(prefix, counter) AVE_TRACE_UID3(prefix, counter)
#define AVE_TRACE_UID3(prefix, counter) prefix##counter

}  // namespace tracing
}  // namespace ave

#endif /* !AVE_BASE_TRACE_H_H_ */
