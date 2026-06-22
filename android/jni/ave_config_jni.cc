/*
 * ave_config_jni.cc
 * Copyright (C) 2026 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <jni.h>

#include "base/android/jni/jni_generator_helper.h"
#include "base/ave_config.h"
#include "jni_headers/base/android/generated_base_jni/AveConfig_jni.h"

namespace ave {
namespace jni {

static void JNI_AveConfig_SetCodecDebugEnabled(JNIEnv* env, jboolean enabled) {
  base::AveConfig::GetInstance().SetCodecDebugEnabled(enabled == JNI_TRUE);
}

}  // namespace jni
}  // namespace ave
