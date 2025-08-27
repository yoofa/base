/*
 * logging_jni.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/android/generated_base_jni/Logging_jni.h"
#include "base/logging.h"
#include "third_party/jni_zero/jni_zero.h"

namespace ave {
namespace jni {

void JNI_Logging_EnableLogThreads(JNIEnv* env) {
  ave::base::LogMessage::LogThreads();
}

void JNI_Logging_EnableLogTimeStamps(JNIEnv* env) {
  ave::base::LogMessage::LogTimestamps();
}

void JNI_Logging_EnableLogToDebugOutput(JNIEnv* env, int32_t nativeSeverity) {
  ave::base::LogMessage::LogToDebug(ave::base::LogSeverity::LS_VERBOSE);
}

void JNI_Logging_Log(JNIEnv* env,
                     int32_t severity,
                     std::string& tag,
                     std::string& message) {
  switch (severity) {
    case 0:
      AVE_LOG(LS_VERBOSE) << tag << ": " << message;
      break;
    case 1:
      AVE_LOG(LS_INFO) << tag << ": " << message;
      break;
    case 2:
      AVE_LOG(LS_WARNING) << tag << ": " << message;
      break;
    case 3:
      AVE_LOG(LS_ERROR) << tag << ": " << message;
      break;
  }
}

}  // namespace jni
}  // namespace ave
