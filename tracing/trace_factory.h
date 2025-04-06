/*
 * trace_factory.h
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#pragma once
#ifndef AVE_BASE_TRACING_TRACE_FACTORY_H_
#define AVE_BASE_TRACING_TRACE_FACTORY_H_

#include <memory>
#include <unordered_set>

#include "trace.h"

namespace ave {
namespace tracing {

/**
 * @brief Factory class for creating tracer instances.
 */
class TraceFactory {
 public:
  /**
   * @brief Create a tracer based on the provided configuration.
   * @param config The trace configuration.
   * @return A shared pointer to the created tracer, or nullptr if creation
   * failed.
   */
  static std::shared_ptr<AbstractTracer> CreateTracer(
      const TraceConfig& config);

  /**
   * @brief Create a file tracer.
   * @param filename The output file path.
   * @param enabledCategories Optional set of categories to enable.
   * @return A shared pointer to the created file tracer.
   */
  static std::shared_ptr<AbstractTracer> CreateFileTracer(
      const std::string& filename,
      const std::unordered_set<std::string>& enabledCategories = {});

  // Add more factory methods for other tracer types as needed
};

}  // namespace tracing
}  // namespace ave

#endif /* !AVE_BASE_TRACING_TRACE_FACTORY_H_ */