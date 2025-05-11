/*
 * file_tracer.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "file_tracer.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

namespace ave {
namespace tracing {

FileTracer::FileTracer(const std::string& filename,
                       const std::unordered_set<std::string>& enabledCategories)
    : filename_(filename),
      initialized_(false),
      enabledCategories_(enabledCategories),
      allCategoriesEnabled_(enabledCategories.empty()) {}

FileTracer::~FileTracer() {
  shutdown();
}

bool FileTracer::initialize() {
  std::lock_guard<std::mutex> lock(fileMutex_);

  if (initialized_) {
    return true;
  }

  file_.open(filename_, std::ios::out | std::ios::trunc);
  if (!file_.is_open()) {
    return false;
  }

  initialized_ = true;

  // Write header
  file_ << "# AVE Trace Log" << std::endl;
  file_ << "# Format: [timestamp] [thread_id] [event_type] [category] [name] "
           "[details]"
        << std::endl;
  file_ << "# Started at: " << getCurrentTimestamp() << std::endl;
  file_.flush();

  return true;
}

void FileTracer::shutdown() {
  std::lock_guard<std::mutex> lock(fileMutex_);

  if (initialized_ && file_.is_open()) {
    file_ << "# Ended at: " << getCurrentTimestamp() << std::endl;
    file_.close();
  }

  initialized_ = false;
}

bool FileTracer::isEnabled() {
  return initialized_ && file_.is_open();
}

bool FileTracer::isCategoryEnabled(std::string_view category) {
  if (!isEnabled()) {
    return false;
  }

  if (allCategoriesEnabled_) {
    return true;
  }

  return enabledCategories_.find(std::string(category)) !=
         enabledCategories_.end();
}

void FileTracer::beginSection(std::string_view category,
                              std::string_view name) {
  std::stringstream ss;
  ss << "[" << getCurrentTimestamp() << "] " << "["
     << std::this_thread::get_id() << "] " << "[BEGIN] " << "[" << category
     << "] " << "[" << name << "]";

  writeToFile(ss.str());
}

void FileTracer::endSection() {
  std::stringstream ss;
  ss << "[" << getCurrentTimestamp() << "] " << "["
     << std::this_thread::get_id() << "] " << "[END]";

  writeToFile(ss.str());
}

void FileTracer::instantEvent(std::string_view category,
                              std::string_view name) {
  std::stringstream ss;
  ss << "[" << getCurrentTimestamp() << "] " << "["
     << std::this_thread::get_id() << "] " << "[INSTANT] " << "[" << category
     << "] " << "[" << name << "]";

  writeToFile(ss.str());
}

void FileTracer::setCounter(std::string_view category,
                            std::string_view name,
                            int64_t value) {
  std::stringstream ss;
  ss << "[" << getCurrentTimestamp() << "] " << "["
     << std::this_thread::get_id() << "] " << "[COUNTER] " << "[" << category
     << "] " << "[" << name << "] " << "value=" << value;

  writeToFile(ss.str());
}

void FileTracer::setCounter(std::string_view category,
                            std::string_view name,
                            double value) {
  std::stringstream ss;
  ss << "[" << getCurrentTimestamp() << "] " << "["
     << std::this_thread::get_id() << "] " << "[COUNTER] " << "[" << category
     << "] " << "[" << name << "] " << "value=" << std::fixed
     << std::setprecision(6) << value;

  writeToFile(ss.str());
}

void FileTracer::beginAsyncEvent(std::string_view category,
                                 std::string_view name,
                                 uint64_t cookie) {
  std::stringstream ss;
  ss << "[" << getCurrentTimestamp() << "] " << "["
     << std::this_thread::get_id() << "] " << "[ASYNC_BEGIN] " << "["
     << category << "] " << "[" << name << "] " << "cookie=" << cookie;

  writeToFile(ss.str());
}

void FileTracer::endAsyncEvent(std::string_view category,
                               std::string_view name,
                               uint64_t cookie) {
  std::stringstream ss;
  ss << "[" << getCurrentTimestamp() << "] " << "["
     << std::this_thread::get_id() << "] " << "[ASYNC_END] " << "[" << category
     << "] " << "[" << name << "] " << "cookie=" << cookie;

  writeToFile(ss.str());
}

void FileTracer::asyncStepEvent(std::string_view category,
                                std::string_view name,
                                uint64_t cookie,
                                std::string_view step_name) {
  std::stringstream ss;
  ss << "[" << getCurrentTimestamp() << "] " << "["
     << std::this_thread::get_id() << "] " << "[ASYNC_STEP] " << "[" << category
     << "] " << "[" << name << "] " << "cookie=" << cookie << " "
     << "step=" << step_name;

  writeToFile(ss.str());
}

std::string FileTracer::getCurrentTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) %
                1000;

  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S") << '.'
     << std::setfill('0') << std::setw(3) << now_ms.count();

  return ss.str();
}

void FileTracer::writeToFile(const std::string& message) {
  std::lock_guard<std::mutex> lock(fileMutex_);

  if (initialized_ && file_.is_open()) {
    file_ << message << std::endl;
    file_.flush();
  }
}

}  // namespace tracing
}  // namespace ave