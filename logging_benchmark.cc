/*
 * logging_benchmark.cc
 * Copyright (C) 2024 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */
#include <string>

#include <benchmark/benchmark.h>

#include "base/logging.h"

// Benchmark for simple string logging
static void BM_SimpleLogging(benchmark::State& state) {
  for (auto _ : state) {
    AVE_LOG(LS_INFO) << "Simple log message";
  }
}
BENCHMARK(BM_SimpleLogging);

// Benchmark for logging with disabled severity level
static void BM_DisabledLogging(benchmark::State& state) {
  ave::base::LogMessage::LogToDebug(ave::base::LogSeverity::LS_NONE);
  for (auto _ : state) {
    AVE_LOG(LS_VERBOSE) << "This should be filtered out";
  }
  ave::base::LogMessage::LogToDebug(ave::base::LogSeverity::LS_VERBOSE);
}
BENCHMARK(BM_DisabledLogging);

// Benchmark for logging with multiple string concatenations
static void BM_StringConcatLogging(benchmark::State& state) {
  const std::string str1 = "First part";
  const std::string str2 = "Second part";
  const std::string str3 = "Third part";

  for (auto _ : state) {
    AVE_LOG(LS_INFO) << str1 << " - " << str2 << " - " << str3;
  }
}
BENCHMARK(BM_StringConcatLogging);

// Benchmark for logging with numbers
static void BM_NumberLogging(benchmark::State& state) {
  for (auto _ : state) {
    AVE_LOG(LS_INFO) << "Value: " << 42 << " float: " << 3.14159;
  }
}
BENCHMARK(BM_NumberLogging);

// Benchmark for logging with timestamps enabled vs disabled
static void BM_LoggingWithTimestamp(benchmark::State& state) {
  ave::base::LogMessage::LogTimestamps(true);
  for (auto _ : state) {
    AVE_LOG(LS_INFO) << "Message with timestamp";
  }
}
BENCHMARK(BM_LoggingWithTimestamp);

static void BM_LoggingWithoutTimestamp(benchmark::State& state) {
  ave::base::LogMessage::LogTimestamps(false);
  for (auto _ : state) {
    AVE_LOG(LS_INFO) << "Message without timestamp";
  }
}
BENCHMARK(BM_LoggingWithoutTimestamp);

// Benchmark for logging with thread ID enabled vs disabled
static void BM_LoggingWithThreadId(benchmark::State& state) {
  ave::base::LogMessage::LogThreads(true);
  for (auto _ : state) {
    AVE_LOG(LS_INFO) << "Message with thread ID";
  }
}
BENCHMARK(BM_LoggingWithThreadId);

static void BM_LoggingWithoutThreadId(benchmark::State& state) {
  ave::base::LogMessage::LogThreads(false);
  for (auto _ : state) {
    AVE_LOG(LS_INFO) << "Message without thread ID";
  }
}
BENCHMARK(BM_LoggingWithoutThreadId);

BENCHMARK_MAIN();
