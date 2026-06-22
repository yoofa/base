/*
 * ave_config.cc
 * Copyright (C) 2026 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/ave_config.h"

namespace ave {
namespace base {

AveConfig& AveConfig::GetInstance() {
  static AveConfig config;
  return config;
}

void AveConfig::SetCodecDebugEnabled(bool enabled) {
  codec_debug_enabled_.store(enabled, std::memory_order_relaxed);
}

bool AveConfig::codec_debug_enabled() const {
  return codec_debug_enabled_.load(std::memory_order_relaxed);
}

}  // namespace base
}  // namespace ave
