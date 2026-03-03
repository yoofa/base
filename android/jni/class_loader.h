/*
 * class_loader.h
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

// Android's FindClass() is tricky because the app-specific ClassLoader is not
// consulted when there is no app-specific frame on the stack (i.e. when called
// from a thread created from native C++ code). These helper functions provide a
// workaround for this.
// http://developer.android.com/training/articles/perf-jni.html#faq_FindClass

#ifndef BASE_ANDROID_JNI_CLASS_LOADER_H_
#define BASE_ANDROID_JNI_CLASS_LOADER_H_

#include <jni.h>

#include "third_party/jni_zero/java_refs.h"

namespace ave {
namespace jni {

/**
 * @brief Initialize the ClassLoader for cross-thread JNI class lookup.
 *        Must be called from JNI_OnLoad after InitGlobalJniVariables.
 * @param env The JNIEnv from the loading thread.
 */
void InitClassLoader(JNIEnv* env);

/**
 * @brief Find a class by name from any thread.
 *        Works even from native-created threads where FindClass would fail.
 * @param env The JNIEnv for the current thread.
 * @param name Fully-qualified class name (e.g. "java/lang/String").
 * @return A local reference to the class.
 */
jni_zero::ScopedJavaLocalRef<jclass> GetClass(JNIEnv* env, const char* name);

}  // namespace jni
}  // namespace ave

#endif  // BASE_ANDROID_JNI_CLASS_LOADER_H_
