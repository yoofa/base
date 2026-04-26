/*
 * frequency.cc - Frequency unit implementation
 *
 * Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 * Adapted for aspect video engine.
 */

#include "base/units/frequency.h"

#include <sstream>

namespace ave {
namespace base {

std::string ToString(Frequency value) {
  std::ostringstream sb;
  if (value.IsPlusInfinity()) {
    sb << "+inf Hz";
  } else if (value.IsMinusInfinity()) {
    sb << "-inf Hz";
  } else if (value.hertz<int64_t>() * 1000 == value.millihertz()) {
    sb << value.hertz<int64_t>() << " Hz";
  } else {
    sb << value.hertz<double>() << " Hz";
  }
  return sb.str();
}

}  // namespace base
}  // namespace ave
