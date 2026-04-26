/*
 * data_rate.cc - Data rate unit class implementation
 * Ported from WebRTC (api/units/data_rate.cc)
 *
 * Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the root of the source tree. An additional
 * intellectual property rights grant can be found in the file PATENTS.
 */

#include "base/units/data_rate.h"

#include <sstream>
#include <string>

namespace ave {
namespace base {

std::string ToString(DataRate value) {
  std::ostringstream oss;
  if (value.IsPlusInfinity()) {
    oss << "+inf bps";
  } else if (value.IsMinusInfinity()) {
    oss << "-inf bps";
  } else if (value.bps() % 1000000 == 0) {
    oss << value.bps() / 1000000 << " Mbps";
  } else if (value.bps() % 1000 == 0) {
    oss << value.bps() / 1000 << " kbps";
  } else {
    oss << value.bps() << " bps";
  }
  return oss.str();
}

}  // namespace base
}  // namespace ave
