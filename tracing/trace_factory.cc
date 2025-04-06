/*
 * trace_factory.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "trace_factory.h"
#include "file_tracer.h"

namespace ave {
namespace tracing {

std::shared_ptr<AbstractTracer> TraceFactory::CreateTracer(
    const TraceConfig& config) {
  switch (config.type) {
    case TraceBackendType::TRACE_TYPE_JSON_FILE:
      return CreateFileTracer(config.json_output_path);

    case TraceBackendType::TRACE_TYPE_PERFETTO_IN_PROCESS:
      // TODO: Implement Perfetto in-process tracer
      return nullptr;

    case TraceBackendType::TRACE_TYPE_PERFETTO_SYSTEM:
      // TODO: Implement Perfetto system tracer
      return nullptr;

    case TraceBackendType::TRACE_TYPE_SYSTRACE:
      // TODO: Implement Systrace tracer
      return nullptr;

    case TraceBackendType::TRACE_TYPE_NONE:
    default:
      return nullptr;
  }
}

std::shared_ptr<AbstractTracer> TraceFactory::CreateFileTracer(
    const std::string& filename,
    const std::unordered_set<std::string>& enabledCategories) {
  return std::make_shared<FileTracer>(filename, enabledCategories);
}

}  // namespace tracing
}  // namespace ave