/*
 * data_size.cc - Data size unit class implementation
 * Ported from WebRTC (api/units/data_size.cc)
 *
 * Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the root of the source tree. An additional
 * intellectual property rights grant can be found in the file PATENTS.
 */

#include "base/units/data_size.h"

#include <sstream>
#include <string>

namespace ave {
namespace base {

std::string ToString(DataSize value) {
  std::ostringstream oss;
  if (value.IsPlusInfinity()) {
    oss << "+inf bytes";
  } else if (value.IsMinusInfinity()) {
    oss << "-inf bytes";
  } else {
    oss << value.bytes() << " bytes";
  }
  return oss.str();
}

}  // namespace base
}  // namespace ave
