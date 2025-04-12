/*
 * trace_factory.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "trace_factory.h"
#include "file_tracer.h"

#if defined(AVE_USE_PERFETTO)
#include "perfetto_tracer.h"
#endif

namespace ave {
namespace tracing {

std::shared_ptr<AbstractTracer> TraceFactory::CreateTracer(
    const TraceConfig& config) {
  switch (config.type) {
    case TraceBackendType::TRACE_TYPE_JSON_FILE:
      return CreateFileTracer(config.json_output_path);

#if defined(AVE_USE_PERFETTO)
    case TraceBackendType::TRACE_TYPE_PERFETTO_IN_PROCESS:
      return CreatePerfettoInProcessTracer(config.perfetto_output_path,
                                           config.perfetto_buffer_kb);

    case TraceBackendType::TRACE_TYPE_PERFETTO_SYSTEM:
      return CreatePerfettoSystemTracer();
#endif

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

#if defined(AVE_USE_PERFETTO)
std::shared_ptr<AbstractTracer> TraceFactory::CreatePerfettoInProcessTracer(
    const std::string& output_path,
    size_t buffer_size_kb,
    const std::unordered_set<std::string>& enabledCategories) {
  return std::make_shared<PerfettoTracer>(output_path, buffer_size_kb,
                                          enabledCategories);
}

std::shared_ptr<AbstractTracer> TraceFactory::CreatePerfettoSystemTracer(
    const std::unordered_set<std::string>& enabledCategories) {
  return std::make_shared<PerfettoTracer>(enabledCategories);
}
#endif

}  // namespace tracing
}  // namespace ave