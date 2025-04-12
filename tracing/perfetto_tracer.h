/*
 * perfetto_tracer.h
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#pragma once
#ifndef AVE_BASE_TRACING_PERFETTO_TRACER_H_
#define AVE_BASE_TRACING_PERFETTO_TRACER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

#include "third_party/perfetto/include/perfetto/tracing/tracing.h"
#include "trace.h"

namespace ave {
namespace tracing {

/**
 * @brief A Perfetto-based tracer implementation.
 *
 * This tracer integrates with the Perfetto tracing system, allowing
 * trace events to be captured by Perfetto's infrastructure.
 */
class PerfettoTracer : public AbstractTracer {
 public:
  /**
   * @brief Constructor for in-process Perfetto tracing
   * @param output_path Path where the trace file will be written
   * @param buffer_size_kb Size of the trace buffer in KB
   * @param enabledCategories List of categories to enable, empty means all
   * categories
   */
  explicit PerfettoTracer(
      std::string output_path,
      size_t buffer_size_kb = 1024,
      std::unordered_set<std::string> enabledCategories = {});

  /**
   * @brief Constructor for system Perfetto tracing
   * @param enabledCategories List of categories to enable, empty means all
   * categories
   */
  explicit PerfettoTracer(
      std::unordered_set<std::string> enabledCategories = {});

  ~PerfettoTracer() override;

  // AbstractTracer implementation
  bool initialize() override;
  void shutdown() override;
  bool isEnabled() override;
  bool isCategoryEnabled(std::string_view category) override;

  void beginSection(std::string_view category, std::string_view name) override;
  void endSection() override;
  void instantEvent(std::string_view category, std::string_view name) override;
  void setCounter(std::string_view category,
                  std::string_view name,
                  int64_t value) override;
  void setCounter(std::string_view category,
                  std::string_view name,
                  double value) override;
  void beginAsyncEvent(std::string_view category,
                       std::string_view name,
                       uint64_t cookie) override;
  void endAsyncEvent(std::string_view category,
                     std::string_view name,
                     uint64_t cookie) override;
  void asyncStepEvent(std::string_view category,
                      std::string_view name,
                      uint64_t cookie,
                      std::string_view step_name) override;

 private:
  // Helper method to register categories with Perfetto
  void registerCategories();

  // Helper method to setup in-process tracing
  bool setupInProcessTracing();

  // Helper method to setup system tracing
  bool setupSystemTracing();

  // Trace configuration
  std::string output_path_;
  size_t buffer_size_kb_;
  std::unordered_set<std::string> enabled_categories_;
  bool all_categories_enabled_;
  bool use_system_backend_;
  bool initialized_;
  std::mutex mutex_;

  // Perfetto session
  std::unique_ptr<perfetto::TracingSession> tracing_session_;
};

}  // namespace tracing
}  // namespace ave

#endif /* !AVE_BASE_TRACING_PERFETTO_TRACER_H_ */
