/*
 * perfetto_tracer.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "perfetto_tracer.h"

#include <fstream>
#include <thread>

#include "perfetto/tracing/core/data_source_config.h"
#include "perfetto/tracing/core/trace_config.h"
#include "perfetto/tracing/track_event.h"
#include "trace_categories.h"

namespace ave {
namespace tracing {

// Thread-local storage for section nesting
thread_local int g_section_nesting = 0;

PerfettoTracer::PerfettoTracer(
    std::string output_path,
    size_t buffer_size_kb,
    std::unordered_set<std::string> enabledCategories)
    : output_path_(std::move(output_path)),
      buffer_size_kb_(buffer_size_kb),
      enabled_categories_(std::move(enabledCategories)),
      all_categories_enabled_(enabledCategories.empty()),
      use_system_backend_(false),
      initialized_(false) {}

PerfettoTracer::PerfettoTracer(
    std::unordered_set<std::string> enabledCategories)
    : buffer_size_kb_(0),
      enabled_categories_(std::move(enabledCategories)),
      all_categories_enabled_(enabledCategories.empty()),
      use_system_backend_(true),
      initialized_(false) {}

PerfettoTracer::~PerfettoTracer() {
  shutdown();
}

bool PerfettoTracer::initialize() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (initialized_) {
    return true;
  }

  // Initialize Perfetto tracing
  perfetto::TracingInitArgs args;
  args.backends = use_system_backend_ ? perfetto::kSystemBackend
                                      : perfetto::kInProcessBackend;
  perfetto::Tracing::Initialize(args);

  // Register track event categories
  registerCategories();

  // Setup tracing based on backend type
  bool success =
      use_system_backend_ ? setupSystemTracing() : setupInProcessTracing();

  if (success) {
    initialized_ = true;
  }

  return initialized_;
}

void PerfettoTracer::shutdown() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (!initialized_) {
    return;
  }

  if (tracing_session_) {
    // Stop the tracing session
    tracing_session_->StopBlocking();

    // For in-process backend, save the trace to a file
    if (!use_system_backend_ && !output_path_.empty()) {
      // Read the trace data
      std::vector<char> trace_data(tracing_session_->ReadTraceBlocking());

      // Write to file
      std::ofstream output(output_path_, std::ios::out | std::ios::binary);
      if (output.is_open()) {
        output.write(trace_data.data(), trace_data.size());
      }
    }

    tracing_session_.reset();
  }

  initialized_ = false;
}

bool PerfettoTracer::isEnabled() {
  return initialized_ && tracing_session_ != nullptr;
}

bool PerfettoTracer::isCategoryEnabled(std::string_view category) {
  if (!isEnabled()) {
    return false;
  }

  if (all_categories_enabled_) {
    return true;
  }

  return enabled_categories_.find(std::string(category)) !=
         enabled_categories_.end();
}

void PerfettoTracer::beginSection(std::string_view category,
                                  std::string_view name) {
  if (!isEnabled() || !isCategoryEnabled(category)) {
    return;
  }

  g_section_nesting++;
  TRACE_EVENT_BEGIN("ave", perfetto::DynamicString{name.data()});
}

void PerfettoTracer::endSection() {
  if (!isEnabled() || g_section_nesting <= 0) {
    return;
  }

  TRACE_EVENT_END("ave");

  // Decrement nesting level
  g_section_nesting--;
}

void PerfettoTracer::instantEvent(std::string_view category,
                                  std::string_view name) {
  if (!isEnabled() || !isCategoryEnabled(category)) {
    return;
  }

  TRACE_EVENT_INSTANT("ave", perfetto::DynamicString{name.data()});
}

void PerfettoTracer::setCounter(std::string_view category,
                                std::string_view name,
                                int64_t value) {
  if (!isEnabled() || !isCategoryEnabled(category)) {
    return;
  }

  // TODO:
}

void PerfettoTracer::setCounter(std::string_view category,
                                std::string_view name,
                                double value) {
  if (!isEnabled() || !isCategoryEnabled(category)) {
    return;
  }

  // TODO:
}

void PerfettoTracer::beginAsyncEvent(std::string_view category,
                                     std::string_view name,
                                     uint64_t cookie) {
  if (!isEnabled() || !isCategoryEnabled(category)) {
    return;
  }

  // Create a track for this async event
  perfetto::Track track(cookie);

  // Begin the slice on this track
  TRACE_EVENT_BEGIN("ave", perfetto::DynamicString{name.data()}, track);
}

void PerfettoTracer::endAsyncEvent(std::string_view category,
                                   std::string_view name,
                                   uint64_t cookie) {
  if (!isEnabled() || !isCategoryEnabled(category)) {
    return;
  }

  // Get the track for this async event
  perfetto::Track track(cookie);

  // End the slice on this track
  TRACE_EVENT_END("ave", track);
}

void PerfettoTracer::asyncStepEvent(std::string_view category,
                                    std::string_view name,
                                    uint64_t cookie,
                                    std::string_view step_name) {
  if (!isEnabled() || !isCategoryEnabled(category)) {
    return;
  }

  // Get the track for this async event
  perfetto::Track track(cookie);

  // Add a step event (we'll implement this as an instant event on the track)
  TRACE_EVENT_INSTANT("ave", perfetto::DynamicString{name.data()}, track);
}

void PerfettoTracer::registerCategories() {
  // Perfetto categories are statically defined via PERFETTO_DEFINE_CATEGORIES
  // at the top of this file. We don't need to do anything here.
}

bool PerfettoTracer::setupInProcessTracing() {
  // Create a new trace config
  perfetto::TraceConfig cfg;

  // Configure the buffer
  auto* buffer = cfg.add_buffers();
  buffer->set_size_kb(buffer_size_kb_);

  // Configure the data source
  auto* ds_cfg = cfg.add_data_sources()->mutable_config();
  ds_cfg->set_name("track_event");

  // Start the tracing session
  tracing_session_ = perfetto::Tracing::NewTrace();
  tracing_session_->Setup(cfg);
  tracing_session_->StartBlocking();

  return true;
}

bool PerfettoTracer::setupSystemTracing() {
  // Create a new trace config
  perfetto::TraceConfig cfg;

  // Configure the data source
  auto* ds_cfg = cfg.add_data_sources()->mutable_config();
  ds_cfg->set_name("track_event");

  // Start the tracing session
  tracing_session_ = perfetto::Tracing::NewTrace(perfetto::kSystemBackend);
  tracing_session_->Setup(cfg);
  tracing_session_->StartBlocking();

  return true;
}

}  // namespace tracing
}  // namespace ave
