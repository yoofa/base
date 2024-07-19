/*
 * time_delta.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "time_delta.h"

#include <array>
#include <sstream>

namespace ave {
namespace base {

std::string ToString(TimeDelta value) {
  std::array<char, 64> buf{};

  std::stringstream ss(buf.data());
  if (value.IsPlusInfinity()) {
    ss << "+inf ms";
  } else if (value.IsMinusInfinity()) {
    ss << "-inf ms";
  } else {
    if (value.us() == 0 || (value.us() % 1000) != 0) {
      ss << value.us() << " us";
    } else if (value.ms() % 1000 != 0) {
      ss << value.ms() << " ms";
    } else {
      ss << value.seconds() << " s";
    }
  }
  return ss.str();
}

}  // namespace base
}  // namespace ave
