/*
 * ave_config.h
 * Copyright (C) 2026 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_BASE_AVE_CONFIG_H_
#define AVE_BASE_AVE_CONFIG_H_

#include <atomic>

#include "base/constructor_magic.h"

namespace ave {
namespace base {

// Process-wide AVE settings. Configure these before creating player or codec
// instances; readers may access them from any thread without propagating
// options through the playback pipeline.
class AveConfig final {
 public:
  static AveConfig& GetInstance();

  void SetCodecDebugEnabled(bool enabled);
  bool codec_debug_enabled() const;

 private:
  AveConfig() = default;

  std::atomic_bool codec_debug_enabled_ = false;

  AVE_DISALLOW_COPY_AND_ASSIGN(AveConfig);
};

}  // namespace base
}  // namespace ave

#endif  // AVE_BASE_AVE_CONFIG_H_
