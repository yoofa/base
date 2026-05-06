/*
 * random.h
 */
#ifndef BASE_RANDOM_H_
#define BASE_RANDOM_H_

#include <cstdint>
#include <random>

namespace ave {
namespace base {

class Random {
 public:
  explicit Random(uint64_t seed) : generator_(seed) {}

  uint32_t Rand(uint32_t low, uint32_t high) {
    std::uniform_int_distribution<uint32_t> dist(low, high);
    return dist(generator_);
  }

  int Rand(int low, int high) {
    std::uniform_int_distribution<int> dist(low, high);
    return dist(generator_);
  }

  uint32_t Rand(uint32_t t) {
    if (t == 0) {
      return 0;
    }
    std::uniform_int_distribution<uint32_t> dist(0, t - 1);
    return dist(generator_);
  }

  uint32_t Rand() { return generator_(); }

  template <typename T>
  T Rand() {
    std::uniform_int_distribution<T> dist(0, std::numeric_limits<T>::max());
    return dist(generator_);
  }

 private:
  std::mt19937 generator_;
};

template <>
inline bool Random::Rand<bool>() {
  std::uniform_int_distribution<int> dist(0, 1);
  return dist(generator_) == 1;
}

}  // namespace base
}  // namespace ave

#endif  // BASE_RANDOM_H_
