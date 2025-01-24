/*
 * statistics_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

// NOLINTBEGIN(modernize-avoid-c-arrays)

#include "statistics.h"

#include <array>
#include <random>
#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace ave {
namespace base {
namespace {

// Helper function to initialize uniform distribution
template <typename T, typename V>
void InitUniform(V& data, T range_min, T range_max) {
  const size_t count = data.capacity();
  std::minstd_rand gen(count);
  std::uniform_real_distribution<T> dis(range_min, range_max);
  for (auto& datum : data) {
    datum = dis(gen);
  }
}

// Helper function to initialize normal distribution
template <typename T, typename V>
void InitNormal(V& data, T mean, T stddev) {
  const size_t count = data.capacity();
  std::minstd_rand gen(count);
  std::normal_distribution<T> dis(mean, stddev);
  for (auto& datum : data) {
    datum = dis(gen);
  }
}

// Used to create compile-time reference constants for variance testing
template <typename T>
class ConstexprStatistics {
 public:
  template <size_t N>
  explicit constexpr ConstexprStatistics(const T (&a)[N])
      : count_(N),
        max_(GetMax(a)),
        min_(GetMin(a)),
        mean_(GetMean(a)),
        m2_(GetM2(a, mean_)),
        pop_variance_(m2_ / N),
        pop_stddev_(sqrt_constexpr(pop_variance_)),
        variance_(m2_ / (N - 1)),
        stddev_(sqrt_constexpr(variance_)) {}

  constexpr int64_t Count() const { return count_; }
  constexpr T Min() const { return min_; }
  constexpr T Max() const { return max_; }
  constexpr double Weight() const { return static_cast<double>(count_); }
  constexpr double Mean() const { return mean_; }
  constexpr double Variance() const { return variance_; }
  constexpr double StdDev() const { return stddev_; }
  constexpr double PopulationVariance() const { return pop_variance_; }
  constexpr double PopulationStdDev() const { return pop_stddev_; }

 private:
  template <size_t N>
  static constexpr T GetMax(const T (&a)[N]) {
    T max = a[0];
    for (size_t i = 1; i < N; ++i) {
      if (a[i] > max)
        max = a[i];
    }
    return max;
  }

  template <size_t N>
  static constexpr T GetMin(const T (&a)[N]) {
    T min = a[0];
    for (size_t i = 1; i < N; ++i) {
      if (a[i] < min)
        min = a[i];
    }
    return min;
  }

  template <size_t N>
  static constexpr double GetMean(const T (&a)[N]) {
    T sum = 0;
    for (size_t i = 0; i < N; ++i) {
      sum += a[i];
    }
    return static_cast<double>(sum) / N;
  }

  template <size_t N>
  static constexpr double GetM2(const T (&a)[N], double mean) {
    double sum = 0;
    for (size_t i = 0; i < N; ++i) {
      double diff = a[i] - mean;
      sum += diff * diff;
    }
    return sum;
  }

  const size_t count_;
  const T max_;
  const T min_;
  const double mean_;
  const double m2_;
  const double pop_variance_;
  const double pop_stddev_;
  const double variance_;
  const double stddev_;
};

// Helper macro for near comparisons
#define TEST_EXPECT_NEAR(expected, actual) \
  EXPECT_NEAR(                             \
      (expected), (actual),                \
      std::abs((expected) *                \
               std::numeric_limits<decltype(expected)>::epsilon() * 8))

#define PRINT_AND_EXPECT_EQ(expected, expr)                              \
  do {                                                                   \
    auto value = (expr);                                                 \
    printf("(%s): %s\n", #expr, std::to_string(value).c_str());          \
    if ((expected) == (expected)) {                                      \
      EXPECT_EQ((expected), (value));                                    \
    }                                                                    \
    EXPECT_EQ((expected) != (expected), value != value); /* nan check */ \
  } while (0)

#define PRINT_AND_EXPECT_NEAR(expected, expr)                   \
  do {                                                          \
    auto ref = (expected);                                      \
    auto value = (expr);                                        \
    printf("(%s): %s\n", #expr, std::to_string(value).c_str()); \
    TEST_EXPECT_NEAR(ref, value);                               \
  } while (0)

template <typename T, typename S>
void Verify(const T& stat, const S& refstat) {
  EXPECT_EQ(refstat.Count(), stat.Count());
  EXPECT_EQ(refstat.Min(), stat.Min());
  EXPECT_EQ(refstat.Max(), stat.Max());
  TEST_EXPECT_NEAR(refstat.Weight(), stat.Weight());
  TEST_EXPECT_NEAR(refstat.Mean(), stat.Mean());
  TEST_EXPECT_NEAR(refstat.Variance(), stat.Variance());
  TEST_EXPECT_NEAR(refstat.StdDev(), stat.StdDev());
  TEST_EXPECT_NEAR(refstat.PopulationVariance(), stat.PopulationVariance());
  TEST_EXPECT_NEAR(refstat.PopulationStdDev(), stat.PopulationStdDev());
}

TEST(StatisticsTest, HighPrecisionSums) {
  static const double simple[] = {1., 2., 3.};

  double rssum = 0;
  for (auto v : simple) {
    rssum += v;
  }
  PRINT_AND_EXPECT_EQ(6., rssum);

  KahanSum<double> ks;
  for (auto v : simple) {
    ks += v;
  }
  PRINT_AND_EXPECT_EQ(6., static_cast<double>(ks));

  NeumaierSum<double> ns;
  for (auto v : simple) {
    ns += v;
  }
  PRINT_AND_EXPECT_EQ(6., static_cast<double>(ns));

  double rs{};
  KahanSum<double> kahan;
  NeumaierSum<double> neumaier;

  // Add 1.
  rs += 1.;
  kahan += 1.;
  neumaier += 1.;

  static constexpr double small_one =
      std::numeric_limits<double>::epsilon() * 0.5;
  // Add lots of small values
  static constexpr int loop = 1000;
  for (int i = 0; i < loop; ++i) {
    rs += small_one;
    kahan += small_one;
    neumaier += small_one;
  }

  // Remove 1.
  rs += -1.;
  kahan += -1.;
  neumaier += -1.;

  const double total_added = small_one * loop;
  printf("total_added: %lg\n", total_added);
  PRINT_AND_EXPECT_EQ(0., rs);                 // normal count fails
  PRINT_AND_EXPECT_EQ(total_added, kahan);     // kahan succeeds
  PRINT_AND_EXPECT_EQ(total_added, neumaier);  // neumaier succeeds

  // Test case where kahan fails and neumaier method succeeds.
  static const double tricky[] = {1e100, 1., -1e100};

  rssum = 0;
  for (auto v : tricky) {
    rssum += v;
  }
  PRINT_AND_EXPECT_EQ(0., rssum);

  KahanSum<double> kssum;
  for (auto v : tricky) {
    kssum += v;
  }
  PRINT_AND_EXPECT_EQ(0., static_cast<double>(kssum));

  NeumaierSum<double> nssum;
  for (auto v : tricky) {
    nssum += v;
  }
  PRINT_AND_EXPECT_EQ(1., static_cast<double>(nssum));
}

// Test basic statistics functionality
TEST(StatisticsTest, BasicStatistics) {
  Statistics<double> stats;

  // Test empty state
  EXPECT_EQ(0, stats.Count());
  EXPECT_EQ(0.0, stats.Variance());
  EXPECT_EQ(0.0, stats.PopulationVariance());

  // Test single value
  stats.Add(1.0);
  EXPECT_EQ(1, stats.Count());
  EXPECT_EQ(1.0, stats.Mean());
  EXPECT_EQ(1.0, stats.Min());
  EXPECT_EQ(1.0, stats.Max());
  EXPECT_EQ(0.0, stats.PopulationVariance());
  EXPECT_EQ(0.0, stats.Variance());

  // Test multiple values
  stats.Add(2.0);
  stats.Add(3.0);
  EXPECT_EQ(3, stats.Count());
  EXPECT_EQ(2.0, stats.Mean());
  EXPECT_EQ(1.0, stats.Min());
  EXPECT_EQ(3.0, stats.Max());
  EXPECT_NEAR(0.6666666666666666, stats.PopulationVariance(), 1e-10);
  EXPECT_NEAR(1.0, stats.Variance(), 1e-10);
}

// Test weighted statistics
TEST(StatisticsTest, WeightedStatistics) {
  Statistics<double> stats(0.5);  // alpha = 0.5

  stats.Add(1.0);
  stats.Add(2.0);
  stats.Add(3.0);

  EXPECT_EQ(3, stats.Count());
  EXPECT_NEAR(2.571428571, stats.Mean(), 1e-6);
  EXPECT_NEAR(0.530612245, stats.PopulationVariance(), 1e-6);
  EXPECT_NEAR(0.816496581, stats.PopulationStdDev(), 1e-6);
}

// Test with large number of samples
TEST(StatisticsTest, LargeSampleSet) {
  constexpr size_t kSampleCount = 1 << 20;  // 1M samples
  constexpr double kRange = 1.0;
  std::vector<double> data(kSampleCount);
  InitUniform(data, -kRange, kRange);

  Statistics<double> stats;
  for (const auto& value : data) {
    stats.Add(value);
  }

  EXPECT_EQ(kSampleCount, static_cast<size_t>(stats.Count()));
  EXPECT_NEAR(0.0, stats.Mean(), 0.01);  // should be close to 0
  EXPECT_LE(stats.Min(), -0.9);          // should be close to -1
  EXPECT_GE(stats.Max(), 0.9);           // should be close to 1
  EXPECT_NEAR(0.33, stats.PopulationVariance(),
              0.01);  // should be close to 1/3
}

// Test with normal distribution
TEST(StatisticsTest, NormalDistribution) {
  constexpr size_t kSampleCount = 1 << 20;  // 1M samples
  constexpr double kMean = 10.0;
  constexpr double kStdDev = 2.0;
  std::vector<double> data(kSampleCount);
  InitNormal(data, kMean, kStdDev);

  Statistics<double> stats;
  for (const auto& value : data) {
    stats.Add(value);
  }

  EXPECT_NEAR(kMean, stats.Mean(), 0.01);
  EXPECT_NEAR(kStdDev, stats.PopulationStdDev(), 0.01);
}

// Test Reset functionality
TEST(StatisticsTest, Reset) {
  Statistics<double> stats;

  stats.Add(1.0);
  stats.Add(2.0);
  EXPECT_EQ(2, stats.Count());

  stats.Reset();
  EXPECT_EQ(0, stats.Count());
  EXPECT_EQ(0.0, stats.Mean());
  EXPECT_EQ(0.0, stats.Variance());
}

TEST(StatisticsTest, MinMaxBounds) {
  static constexpr double one[] = {1.};

  PRINT_AND_EXPECT_EQ(std::numeric_limits<double>::infinity(),
                      one[0]);  // min of empty range

  PRINT_AND_EXPECT_EQ(-std::numeric_limits<double>::infinity(),
                      one[0]);  // max of empty range

  static constexpr int un[] = {1};

  PRINT_AND_EXPECT_EQ(std::numeric_limits<int>::max(),
                      un[0]);  // min of empty range

  PRINT_AND_EXPECT_EQ(std::numeric_limits<int>::min(),
                      un[0]);  // max of empty range

  double nan_array[] = {std::numeric_limits<double>::quiet_NaN(),
                        std::numeric_limits<double>::quiet_NaN(),
                        std::numeric_limits<double>::quiet_NaN()};

  Statistics<double> s(nan_array);

  PRINT_AND_EXPECT_EQ(std::numeric_limits<double>::infinity(), s.Min());
  PRINT_AND_EXPECT_EQ(-std::numeric_limits<double>::infinity(), s.Max());
}

TEST(StatisticsTest, MinMaxSimpleArray) {
  static constexpr double ary[] = {-1.5, 1.5, -2.5, 2.5};

  PRINT_AND_EXPECT_EQ(-2.5, *std::min_element(ary, ary + 4));

  PRINT_AND_EXPECT_EQ(2.5, *std::max_element(ary, ary + 4));

  static constexpr int ray[] = {-1, 1, -2, 2};

  PRINT_AND_EXPECT_EQ(-2, *std::min_element(ray, ray + 4));

  PRINT_AND_EXPECT_EQ(2, *std::max_element(ray, ray + 4));
}

TEST(StatisticsTest, Sqrt) {
  // Check doubles
  PRINT_AND_EXPECT_EQ(std::numeric_limits<double>::infinity(),
                      std::sqrt(std::numeric_limits<double>::infinity()));

  PRINT_AND_EXPECT_EQ(std::numeric_limits<double>::quiet_NaN(),
                      std::sqrt(-std::numeric_limits<double>::infinity()));

  PRINT_AND_EXPECT_NEAR(std::sqrt(std::numeric_limits<double>::epsilon()),
                        std::sqrt(std::numeric_limits<double>::epsilon()));

  PRINT_AND_EXPECT_EQ(3., std::sqrt(9.));

  PRINT_AND_EXPECT_EQ(0., std::sqrt(0.));

  PRINT_AND_EXPECT_EQ(std::numeric_limits<double>::quiet_NaN(), std::sqrt(-1.));

  PRINT_AND_EXPECT_EQ(std::numeric_limits<double>::quiet_NaN(),
                      std::sqrt(std::numeric_limits<double>::quiet_NaN()));

  // Check floats
  PRINT_AND_EXPECT_EQ(std::numeric_limits<float>::infinity(),
                      std::sqrt(std::numeric_limits<float>::infinity()));

  PRINT_AND_EXPECT_EQ(std::numeric_limits<float>::quiet_NaN(),
                      std::sqrt(-std::numeric_limits<float>::infinity()));

  PRINT_AND_EXPECT_NEAR(std::sqrt(std::numeric_limits<float>::epsilon()),
                        std::sqrt(std::numeric_limits<float>::epsilon()));

  PRINT_AND_EXPECT_EQ(2.f, std::sqrt(4.f));

  PRINT_AND_EXPECT_EQ(0.f, std::sqrt(0.f));

  PRINT_AND_EXPECT_EQ(std::numeric_limits<float>::quiet_NaN(), std::sqrt(-1.f));

  PRINT_AND_EXPECT_EQ(std::numeric_limits<float>::quiet_NaN(),
                      std::sqrt(std::numeric_limits<float>::quiet_NaN()));
}

// NOLINTBEGIN(modernize-avoid-c-arrays)

TEST(StatisticsTest, StatReference) {
  static constexpr double data[] = {0.1, -0.1, 0.2, -0.3};
  static constexpr ConstexprStatistics<double> rstat(data);
  static constexpr Statistics<double> stat(data);

  Verify(stat, rstat);
}

// NOLINTEND(modernize-avoid-c-arrays)

TEST(StatisticsTest, StatVariableAlpha) {
  constexpr size_t kTestSize = 1 << 20;
  std::vector<double> data(kTestSize);
  std::vector<double> alpha(kTestSize);

  InitUniform(data, -1., 1.);
  InitUniform(alpha, .95, .99);

  Statistics<double> stat;
  ReferenceStatistics<double> rstat;

  static_assert(std::is_trivially_copyable<decltype(stat)>::value,
                "basic statistics must be trivially copyable");

  for (size_t i = 0; i < kTestSize; ++i) {
    rstat.SetAlpha(alpha[i]);
    rstat.Add(data[i]);

    stat.SetAlpha(alpha[i]);
    stat.Add(data[i]);
  }

  printf("statistics: %s\n", stat.ToString().c_str());
  printf("ref statistics: %s\n", rstat.ToString().c_str());
  Verify(stat, rstat);
}

TEST(StatisticsTest, StatVector) {
  using data_t = std::tuple<double, double>;
  using covariance_t = std::tuple<double, double, double, double>;
  using covariance_ut_t = std::tuple<double, double, double>;

  constexpr size_t kTestSize = 1 << 20;
  std::vector<data_t> data(kTestSize);

  InitUniform(data, -1., 1.);

  std::cout << "sample data[0]: " << std::get<0>(data[0]) << ", "
            << std::get<1>(data[0]) << "\n";

  Statistics<data_t, data_t, data_t, double, double, std::multiplies<data_t>>
      stat;
  Statistics<data_t, data_t, data_t, double, covariance_t,
             std::multiplies<data_t>>
      stat_outer;
  Statistics<data_t, data_t, data_t, double, covariance_ut_t,
             std::multiplies<data_t>>
      stat_outer_ut;

  using pair_t = std::pair<double, double>;
  std::vector<pair_t> pairs(kTestSize);
  InitUniform(pairs, -1., 1.);
  Statistics<pair_t, pair_t, pair_t, double, double, std::multiplies<pair_t>>
      stat_pair;

  using array_t = std::array<double, 2>;
  using array_covariance_ut_t = std::array<double, 3>;
  std::vector<array_t> arrays(kTestSize);
  InitUniform(arrays, -1., 1.);
  Statistics<array_t, array_t, array_t, double, double,
             std::multiplies<array_t>>
      stat_array;
  Statistics<array_t, array_t, array_t, double, array_covariance_ut_t,
             std::multiplies<array_t>>
      stat_array_ut;

  for (size_t i = 0; i < kTestSize; ++i) {
    stat.Add(data[i]);
    stat_outer.Add(data[i]);
    stat_outer_ut.Add(data[i]);
    stat_pair.Add(pairs[i]);
    stat_array.Add(arrays[i]);
    stat_array_ut.Add(arrays[i]);
  }

  static_assert(std::is_trivially_copyable<decltype(stat_array)>::value,
                "array based inner product not trivially copyable");
  static_assert(std::is_trivially_copyable<decltype(stat_array_ut)>::value,
                "array based inner product not trivially copyable");

  const double variance = stat.PopulationVariance();
  EXPECT_NEAR(variance,
              std::get<0>(stat_outer.PopulationVariance()) +
                  std::get<3>(stat_outer.PopulationVariance()),
              variance * std::numeric_limits<double>::epsilon() * 128);

  PRINT_AND_EXPECT_NEAR(std::get<1>(stat_outer.PopulationVariance()),
                        std::get<2>(stat_outer.PopulationVariance()));

  PRINT_AND_EXPECT_NEAR(std::get<0>(stat_outer.PopulationVariance()),
                        std::get<0>(stat_outer_ut.PopulationVariance()));
  PRINT_AND_EXPECT_NEAR(std::get<1>(stat_outer.PopulationVariance()),
                        std::get<1>(stat_outer_ut.PopulationVariance()));
  PRINT_AND_EXPECT_NEAR(std::get<3>(stat_outer.PopulationVariance()),
                        std::get<2>(stat_outer_ut.PopulationVariance()));

  PRINT_AND_EXPECT_EQ(variance, stat_pair.PopulationVariance());

  printf("statistics_inner: %s\n", stat.ToString().c_str());
  printf("statistics_outer: %s\n", stat_outer.ToString().c_str());
  printf("statistics_outer_ut: %s\n", stat_outer_ut.ToString().c_str());
}

TEST(StatisticsTest, StatLinearFit) {
  LinearLeastSquaresFit<double> fit;

  static_assert(std::is_trivially_copyable<decltype(fit)>::value,
                "LinearLeastSquaresFit must be trivially copyable");

  using array_t = std::array<double, 2>;
  array_t data{0.0, 1.5};

  for (size_t i = 0; i < 10; ++i) {
    fit.Add(data);
    data[0] += 0.1;
    data[1] += 0.2;
  }

  // Check the y line equation
  {
    double a, b, r2;
    fit.ComputeYLine(a, b, r2);
    printf("y line - a:%lf  b:%lf  r2:%lf\n", a, b, r2);
    PRINT_AND_EXPECT_NEAR(1.5, a);   // y intercept
    PRINT_AND_EXPECT_NEAR(2.0, b);   // y slope
    PRINT_AND_EXPECT_NEAR(1.0, r2);  // correlation coefficient

    double ac, bc, r2c;
    ComputeYLineFromStatistics(ac, bc, r2c, std::get<0>(fit.Mean()),
                               std::get<1>(fit.Mean()),
                               std::get<0>(fit.PopulationVariance()),
                               std::get<1>(fit.PopulationVariance()),
                               std::get<2>(fit.PopulationVariance()));

    EXPECT_EQ(a, ac);
    EXPECT_EQ(b, bc);
    EXPECT_EQ(r2, r2c);

    TEST_EXPECT_NEAR(1.9, fit.GetYFromX(0.2));
    TEST_EXPECT_NEAR(0.2, fit.GetXFromY(1.9));
    TEST_EXPECT_NEAR(1.0, fit.GetR2());
  }

  // Check the x line equation
  {
    double a, b, r2;
    fit.ComputeXLine(a, b, r2);
    printf("x line - a:%lf  b:%lf  r2:%lf\n", a, b, r2);
    PRINT_AND_EXPECT_NEAR(-0.75, a);  // x intercept
    PRINT_AND_EXPECT_NEAR(0.5, b);    // x slope
    PRINT_AND_EXPECT_NEAR(1.0, r2);   // correlation coefficient
  }
}

TEST(StatisticsTest, StatLinearFitNoise) {
  using array_t = std::array<double, 2>;
  LinearLeastSquaresFit<double> fit;

  constexpr size_t kElements = 1000;
  array_t incr{1. / kElements, 1. / kElements};

  std::vector<array_t> noise(kElements);
  InitNormal(noise, 0., 1.);

  for (int i = 0; i < 30; ++i) {
    const double stddev = (i <= 10)   ? i / 1000.
                          : (i <= 20) ? (i - 9) / 100.
                                      : (i - 19) / 10.;
    fit.Reset();

    for (size_t j = 0; j < kElements; ++j) {
      array_t data = {j * incr[0] + noise[j][0] * stddev,
                      j * incr[1] + noise[j][1] * stddev};
      fit.Add(data);
    }

    double a, b, r2;
    fit.ComputeYLine(a, b, r2);
    printf("stddev: %lf y line - N:%lld a:%lf  b:%lf  r2:%lf\n", stddev,
           fit.Count(), a, b, r2);
  }
}

}  // namespace
}  // namespace base
}  // namespace ave

// NOLINTEND(modernize-avoid-c-arrays)