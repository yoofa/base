/*
 * timestamp.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "timestamp.h"

#include <sstream>
#include <string>

namespace ave {
namespace base {

std::string ToString(Timestamp value) {
  std::ostringstream oss;
  if (value.IsPlusInfinity()) {
    oss << "+inf ms";
  } else if (value.IsMinusInfinity()) {
    oss << "-inf ms";
  } else {
    if (value.us() == 0 || (value.us() % 1000) != 0) {
      oss << value.us() << " us";
    } else if (value.ms() % 1000 != 0) {
      oss << value.ms() << " ms";
    } else {
      oss << value.seconds() << " s";
    }
  }
  return oss.str();
}

}  // namespace base
}  // namespace ave
