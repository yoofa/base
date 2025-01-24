/*
 * statistics_benchmark.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <random>
#include <vector>

#include "base/numerics/statistics.h"
#include "benchmark/benchmark.h"

namespace ave {
namespace base {
namespace {

template <typename T>
void InitUniform(std::vector<T>& data, T range_min, T range_max) {
  const size_t count = data.capacity();
  std::minstd_rand gen(count);
  std::uniform_real_distribution<T> dis(range_min, range_max);
  for (auto& datum : data) {
    datum = dis(gen);
  }
}

template <typename Stats>
void BM_MeanVariance(benchmark::State& state, int iter_limit, int alpha_limit) {
  const float alpha = 1.f - alpha_limit * std::numeric_limits<float>::epsilon();
  Stats stat(alpha);
  using T = decltype(stat.Min());
  constexpr size_t count = 1 << 20;  // 1M samples
  constexpr T range = 1.;
  std::vector<T> data(count);
  InitUniform(data, -range, range);

  int iters = 0;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(data.data());
    for (const auto& datum : data) {
      stat.Add(datum);
    }
    benchmark::ClobberMemory();
    if (++iters % iter_limit == 0) {
      printf("%d>  alpha:%f  mean:%.17g  variance:%.17g\n", iters, alpha,
             static_cast<double>(stat.Mean()),
             static_cast<double>(stat.PopulationVariance()));
      stat.Reset();
    }
  }
  state.SetComplexityN(count);
}

// Test case 1: float statistics with alpha = 1
static constexpr int kFloatIterLimit = 8;
static constexpr int kAlphaEqualsOneLimit = 0;

// Benchmark running float
static void BM_MeanVariance_float_float_float(benchmark::State& state) {
  BM_MeanVariance<Statistics<float, float, float>>(state, kFloatIterLimit,
                                                   kAlphaEqualsOneLimit);
}
BENCHMARK(BM_MeanVariance_float_float_float);

// Benchmark running double
static void BM_MeanVariance_float_double_double(benchmark::State& state) {
  BM_MeanVariance<Statistics<float, double, double>>(state, kFloatIterLimit,
                                                     kAlphaEqualsOneLimit);
}
BENCHMARK(BM_MeanVariance_float_double_double);

// Benchmark running float + Kahan
static void BM_MeanVariance_float_float_Kahan(benchmark::State& state) {
  BM_MeanVariance<Statistics<float, float, KahanSum<float>>>(
      state, kFloatIterLimit, kAlphaEqualsOneLimit);
}
BENCHMARK(BM_MeanVariance_float_float_Kahan);

// Benchmark running float + Neumaier
static void BM_MeanVariance_float_float_Neumaier(benchmark::State& state) {
  BM_MeanVariance<Statistics<float, float, NeumaierSum<float>>>(
      state, kFloatIterLimit, kAlphaEqualsOneLimit);
}
BENCHMARK(BM_MeanVariance_float_float_Neumaier);

// Test case 2: float statistics with alpha near 1
static constexpr int kFloatOverflowIterLimit = 32;
static constexpr int kAlphaSafeUpperBoundLimit = 32;

// Benchmark running float at alpha
static void BM_MeanVariance_float_float_float_alpha(benchmark::State& state) {
  BM_MeanVariance<Statistics<float, float, float>>(
      state, kFloatOverflowIterLimit, kAlphaSafeUpperBoundLimit);
}
BENCHMARK(BM_MeanVariance_float_float_float_alpha);

// Benchmark running double at alpha
static void BM_MeanVariance_float_double_double_alpha(benchmark::State& state) {
  BM_MeanVariance<Statistics<float, double, double>>(
      state, kFloatOverflowIterLimit, kAlphaSafeUpperBoundLimit);
}
BENCHMARK(BM_MeanVariance_float_double_double_alpha);

}  // namespace
}  // namespace base
}  // namespace ave

BENCHMARK_MAIN();