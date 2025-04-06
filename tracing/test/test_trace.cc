/*
 * test_trace.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "../../attributes.h"
#include "../file_tracer.h"
#include "../trace.h"
#include "../trace_factory.h"

// Define ENABLE_TRACING to enable tracing
#define ENABLE_TRACING

using namespace ave;
using namespace ave::tracing;

// Example function that performs some work and uses tracing
void performTask(const std::string& task_name, int iterations) {
  // Create a trace scope for this function
  TRACE_SCOPE_CATEGORY("tasks", task_name);

  // Log the number of iterations as a counter
  TRACE_COUNTER_CATEGORY("tasks", "iterations", iterations);

  // Simulate work with iterations
  for (int i = 0; i < iterations; ++i) {
    // Create a trace scope for each iteration
    TRACE_SCOPE_CATEGORY("tasks", ("iteration_" + std::to_string(i)));

    // Log the current iteration as a counter
    TRACE_COUNTER_CATEGORY("tasks", "current_iteration", i);

    // Log an instant event
    TRACE_EVENT_CATEGORY("tasks", ("processing_" + std::to_string(i)));

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // Log completion event
  TRACE_EVENT_CATEGORY("tasks", (task_name + "_completed"));
}

// Example of using async events
void performAsyncOperation() {
  // Generate a unique ID for this async operation
  auto async_id AVE_MAYBE_UNUSED =
      std::hash<std::thread::id>{}(std::this_thread::get_id());

  // Start the async operation
  TRACE_ASYNC_BEGIN_CATEGORY("async", "long_operation", async_id);

  // First part of the async operation
  {
    TRACE_SCOPE_CATEGORY("async", "part1");
    TRACE_ASYNC_STEP_CATEGORY("async", "long_operation", async_id, "started");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Middle part of the async operation
  {
    TRACE_SCOPE_CATEGORY("async", "part2");
    TRACE_ASYNC_STEP_CATEGORY("async", "long_operation", async_id, "middle");
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
  }

  // Final part of the async operation
  {
    TRACE_SCOPE_CATEGORY("async", "part3");
    TRACE_ASYNC_STEP_CATEGORY("async", "long_operation", async_id, "finishing");
    std::this_thread::sleep_for(std::chrono::milliseconds(75));
  }

  // End the async operation
  TRACE_ASYNC_END_CATEGORY("async", "long_operation", async_id);
}

// Example of using the default category
void simpleFunction() {
  TRACE_SCOPE("simpleFunction");

  TRACE_EVENT("simple_event");
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  TRACE_COUNTER("simple_counter", 42);
  std::this_thread::sleep_for(std::chrono::milliseconds(30));

  // 如果需要浮点计数器
  TRACE_COUNTER("float_counter", 3.14);
}

// Example of multi-threaded tracing
void threadedExample() {
  TRACE_SCOPE_CATEGORY("threading", "threadedExample");

  // Create and start threads
  std::thread t1([]() {
    TRACE_SCOPE_CATEGORY("threading", "thread1");
    performTask("thread1_task", 3);
  });

  std::thread t2([]() {
    TRACE_SCOPE_CATEGORY("threading", "thread2");
    performTask("thread2_task", 2);
  });

  // Wait for threads to complete
  t1.join();
  t2.join();

  TRACE_EVENT_CATEGORY("threading", "all_threads_completed");
}

int main(int argc, char* argv[]) {
  std::cout << "Trace Test Example" << std::endl;

  // Initialize tracing with a file tracer
  TraceConfig config;
  config.type = TraceBackendType::TRACE_TYPE_JSON_FILE;
  config.json_output_path = "trace_output.log";

  TRACE_INITIALIZE(config);
  std::cout << "Tracing initialized. Output file: " << config.json_output_path
            << std::endl;

  // Run example functions
  TRACE_SCOPE("main");

  std::cout << "Running simple function..." << std::endl;
  simpleFunction();

  std::cout << "Running task with 5 iterations..." << std::endl;
  performTask("main_task", 5);

  std::cout << "Running async operation..." << std::endl;
  performAsyncOperation();

  std::cout << "Running threaded example..." << std::endl;
  threadedExample();

  // Shutdown tracing
  TRACE_SHUTDOWN();
  std::cout << "Tracing completed and shut down." << std::endl;

  return 0;
}
