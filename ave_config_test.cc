/*
 * ave_config_test.cc
 * Copyright (C) 2026 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <cassert>

#include "base/ave_config.h"

int main(int /*argc*/, char const* /*argv*/[]) {
  auto& config = ave::base::AveConfig::GetInstance();
  assert(!config.codec_debug_enabled());

  config.SetCodecDebugEnabled(true);
  assert(config.codec_debug_enabled());

  config.SetCodecDebugEnabled(false);
  assert(!config.codec_debug_enabled());

  assert(!config.demuxer_logs_enabled());
  config.SetDemuxerLogsEnabled(true);
  assert(config.demuxer_logs_enabled());

  config.SetDemuxerLogsEnabled(false);
  assert(!config.demuxer_logs_enabled());
  return 0;
}
