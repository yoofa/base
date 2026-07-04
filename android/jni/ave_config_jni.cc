/*
 * ave_config_jni.cc
 * Copyright (C) 2026 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <jni.h>

#include "base/ave_config.h"
#include "jni_headers/base/android/generated_base_jni/AveConfig_jni.h"

namespace ave {
namespace jni {

static void JNI_AveConfig_SetCodecDebugEnabled(JNIEnv* /*env*/, bool enabled) {
  base::AveConfig::GetInstance().SetCodecDebugEnabled(enabled);
}

static void JNI_AveConfig_SetDemuxerLogsEnabled(JNIEnv* /*env*/, bool enabled) {
  base::AveConfig::GetInstance().SetDemuxerLogsEnabled(enabled);
}

static void JNI_AveConfig_SetRenderSyncLogsEnabled(JNIEnv* /*env*/,
                                                   bool enabled) {
  base::AveConfig::GetInstance().SetRenderSyncLogsEnabled(enabled);
}

}  // namespace jni
}  // namespace ave
