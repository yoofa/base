/*
 * time_delta.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "time_delta.h"

#include <sstream>

namespace ave {
namespace base {

std::string ToString(TimeDelta value) {
  char buf[64];
  std::stringstream ss(buf);
  if (value.IsPlusInfinity()) {
    ss << "+inf ms";
  } else if (value.IsMinusInfinity()) {
    ss << "-inf ms";
  } else {
    if (value.us() == 0 || (value.us() % 1000) != 0)
      ss << value.us() << " us";
    else if (value.ms() % 1000 != 0)
      ss << value.ms() << " ms";
    else
      ss << value.seconds() << " s";
  }
  return ss.str();
}

}  // namespace base
}  // namespace ave
