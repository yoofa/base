/*
 * file_tracer.h
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#pragma once
#ifndef AVE_BASE_TRACING_FILE_TRACER_H_
#define AVE_BASE_TRACING_FILE_TRACER_H_

#include <chrono>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_set>
#include "trace.h"

namespace ave {
namespace tracing {

/**
 * @brief A simple file-based tracer implementation.
 *
 * This tracer writes trace events to a text file in a simple format.
 * It's primarily intended for debugging and as an example implementation.
 */
class FileTracer : public AbstractTracer {
 public:
  /**
   * @brief Constructor
   * @param filename Path to the output file
   * @param enabledCategories List of categories to enable, empty means all
   * categories
   */
  explicit FileTracer(
      const std::string& filename,
      const std::unordered_set<std::string>& enabledCategories = {});

  ~FileTracer() override;

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
  // Helper method to get current timestamp
  static std::string getCurrentTimestamp();

  // Helper method to write to file with mutex protection
  void writeToFile(const std::string& message);

  std::string filename_;
  std::ofstream file_;
  std::mutex fileMutex_;
  bool initialized_;
  std::unordered_set<std::string> enabledCategories_;
  bool allCategoriesEnabled_;
};

}  // namespace tracing
}  // namespace ave

#endif /* !AVE_BASE_TRACING_FILE_TRACER_H_ */