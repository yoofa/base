/*
 * test_perfetto_trace.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "../../attributes.h"
#include "../perfetto_tracer.h"
#include "../trace.h"
#include "../trace_factory.h"

// Define ENABLE_TRACING to enable tracing
#define ENABLE_TRACING

using namespace ave;
using namespace ave::tracing;

// Example function that performs some work and uses tracing
void performTask(const std::string& task_name, int iterations) {
  // Create a trace scope for this function
  AVE_TRACE_SCOPE_CATEGORY("tasks", task_name);

  // Log the number of iterations as a counter
  AVE_TRACE_COUNTER_CATEGORY("tasks", "iterations", iterations);

  // Simulate work with iterations
  for (int i = 0; i < iterations; ++i) {
    // Create a trace scope for each iteration
    AVE_TRACE_SCOPE_CATEGORY("tasks", ("iteration_" + std::to_string(i)));

    // Log the current iteration as a counter
    AVE_TRACE_COUNTER_CATEGORY("tasks", "current_iteration", i);

    // Log an instant event
    AVE_TRACE_EVENT_CATEGORY("tasks", ("processing_" + std::to_string(i)));

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // Log completion event
  AVE_TRACE_EVENT_CATEGORY("tasks", (task_name + "_completed"));
}

int main(int argc, char* argv[]) {
  std::cout << "Perfetto Trace Test Example" << std::endl;

  // Initialize tracing with a Perfetto in-process tracer
  TraceConfig config;
  config.type = TraceBackendType::TRACE_TYPE_PERFETTO_IN_PROCESS;
  config.perfetto_output_path = "perfetto_trace.pftrace";
  config.perfetto_buffer_kb = 2048;

  AVE_TRACE_INITIALIZE(config);
  std::cout << "Tracing initialized. Output file: "
            << config.perfetto_output_path << std::endl;

  // Run example functions
  AVE_TRACE_SCOPE("main");

  std::cout << "Running task with 5 iterations..." << std::endl;
  performTask("main_task", 5);

  // Shutdown tracing
  AVE_TRACE_SHUTDOWN();
  std::cout << "Tracing completed and shut down." << std::endl;

  std::cout
      << "Trace file can be viewed with Perfetto UI at https://ui.perfetto.dev"
      << std::endl;

  return 0;
}