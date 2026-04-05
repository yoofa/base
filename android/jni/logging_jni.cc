/*
 * logging_jni.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <jni.h>
#include <string>

#include "base/logging.h"
#include "jni_headers/base/android/generated_base_jni/Logging_jni.h"

namespace ave {
namespace jni {

static void JNI_Logging_EnableLogToDebugOutput(jint nativeSeverity) {
  if (nativeSeverity >= 0 &&
      nativeSeverity <= static_cast<int>(ave::base::LS_NONE)) {
    ave::base::LogMessage::LogToDebug(
        static_cast<ave::base::LogSeverity>(nativeSeverity));
  }
}

static void JNI_Logging_EnableLogThreads() {
  ave::base::LogMessage::LogThreads(true);
}

static void JNI_Logging_EnableLogTimeStamps() {
  ave::base::LogMessage::LogTimestamps(true);
}

static void JNI_Logging_Log(jint severity,
                            std::string& tag,
                            std::string& message) {
  auto sev = static_cast<ave::base::LogSeverity>(severity);
  ave::base::LogMessage(__FILE__, __LINE__, sev).stream()
      << tag << ": " << message;
}

}  // namespace jni
}  // namespace ave
