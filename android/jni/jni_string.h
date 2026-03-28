/*
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_ANDROID_JNI_JNI_STRING_H_
#define BASE_ANDROID_JNI_JNI_STRING_H_

#include <jni.h>

#include <string>

#include "third_party/jni_zero/jni_zero.h"

// Provides jni_zero::FromJniType<std::string> and ToJniType<std::string>
// specializations so that @JniType("std::string") annotations in Java proxy
// interfaces automatically convert Java String <-> C++ std::string.
//
// Include this header (via jni_generator_helper.h) before any generated
// _jni.h file that uses @JniType("std::string").
namespace jni_zero {

/// Converts a Java String (boxed as jobject) to a UTF-8 std::string.
template <>
inline std::string FromJniType<std::string>(
    JNIEnv* env,
    const JavaRef<jobject>& j_object) {
  if (!j_object) {
    return std::string();
  }
  const jstring j_str = static_cast<jstring>(j_object.obj());
  const char* chars = env->GetStringUTFChars(j_str, nullptr);
  if (!chars) {
    return std::string();
  }
  std::string result(chars);
  env->ReleaseStringUTFChars(j_str, chars);
  return result;
}

/// Converts a UTF-8 std::string to a Java String (returned as ScopedJavaLocalRef<jobject>).
template <>
inline ScopedJavaLocalRef<jobject> ToJniType<std::string>(
    JNIEnv* env,
    const std::string& str) {
  jstring j_str = env->NewStringUTF(str.c_str());
  return ScopedJavaLocalRef<jobject>::Adopt(
      env, static_cast<jobject>(j_str));
}

}  // namespace jni_zero

#endif  // BASE_ANDROID_JNI_JNI_STRING_H_
