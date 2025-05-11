/*
 * trace.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "trace.h"
#include <mutex>
#include "trace_factory.h"

namespace ave {
namespace tracing {

// Static member initialization
std::shared_ptr<AbstractTracer> Trace::s_tracer = nullptr;
bool Trace::s_enabled = false;
std::mutex s_trace_mutex;

bool Trace::initialize(const TraceConfig& config) {
  std::lock_guard<std::mutex> lock(s_trace_mutex);

  // Clean up any existing tracer
  if (s_tracer) {
    s_tracer->shutdown();
    s_tracer = nullptr;
  }

  // Create the appropriate tracer using the factory
  s_tracer = TraceFactory::CreateTracer(config);

  // Initialize the tracer
  if (s_tracer) {
    s_enabled = s_tracer->initialize() && s_tracer->isEnabled();
    return s_enabled;
  }
  if (config.type == TraceBackendType::TRACE_TYPE_NONE) {
    s_enabled = false;
    return true;  // Successfully disabled tracing
  }

  return false;  // Failed to create tracer
}

void Trace::initialize(std::shared_ptr<AbstractTracer> tracer_impl) {
  std::lock_guard<std::mutex> lock(s_trace_mutex);

  // Clean up any existing tracer
  if (s_tracer) {
    s_tracer->shutdown();
  }

  s_tracer = tracer_impl;

  if (s_tracer) {
    s_enabled = s_tracer->initialize() && s_tracer->isEnabled();
  } else {
    s_enabled = false;
  }
}

void Trace::shutdown() {
  std::lock_guard<std::mutex> lock(s_trace_mutex);

  if (s_tracer) {
    s_tracer->shutdown();
    s_tracer = nullptr;
  }

  s_enabled = false;
}

bool Trace::isEnabled() {
  return s_enabled;
}

bool Trace::isCategoryEnabled(std::string_view category) {
  if (!s_enabled || !s_tracer) {
    return false;
  }

  return s_tracer->isCategoryEnabled(category);
}

void Trace::beginSection(std::string_view category, std::string_view name) {
  if (!s_enabled || !s_tracer) {
    return;
  }

  s_tracer->beginSection(category, name);
}

void Trace::endSection() {
  if (!s_enabled || !s_tracer) {
    return;
  }

  s_tracer->endSection();
}

void Trace::instantEvent(std::string_view category, std::string_view name) {
  if (!s_enabled || !s_tracer) {
    return;
  }

  s_tracer->instantEvent(category, name);
}

void Trace::setCounter(std::string_view category,
                       std::string_view name,
                       int64_t value) {
  if (!s_enabled || !s_tracer) {
    return;
  }

  s_tracer->setCounter(category, name, value);
}

void Trace::setCounter(std::string_view category,
                       std::string_view name,
                       double value) {
  if (!s_enabled || !s_tracer) {
    return;
  }

  s_tracer->setCounter(category, name, value);
}

void Trace::setCounter(std::string_view category,
                       std::string_view name,
                       int value) {
  setCounter(category, name, static_cast<int64_t>(value));
}

void Trace::beginAsyncEvent(std::string_view category,
                            std::string_view name,
                            uint64_t cookie) {
  if (!s_enabled || !s_tracer) {
    return;
  }

  s_tracer->beginAsyncEvent(category, name, cookie);
}

void Trace::endAsyncEvent(std::string_view category,
                          std::string_view name,
                          uint64_t cookie) {
  if (!s_enabled || !s_tracer) {
    return;
  }

  s_tracer->endAsyncEvent(category, name, cookie);
}

void Trace::asyncStepEvent(std::string_view category,
                           std::string_view name,
                           uint64_t cookie,
                           std::string_view step_name) {
  if (!s_enabled || !s_tracer) {
    return;
  }

  s_tracer->asyncStepEvent(category, name, cookie, step_name);
}

}  // namespace tracing
}  // namespace ave
