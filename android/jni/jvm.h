/*
 * jvm.h
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_ANDROID_JNI_JVM_H_
#define BASE_ANDROID_JNI_JVM_H_

#include <jni.h>

namespace ave {
namespace jni {

/**
 * @brief Initialize global JNI variables. Must be called from JNI_OnLoad.
 * @param jvm The Java Virtual Machine pointer.
 * @return JNI version on success, -1 on failure.
 */
jint InitGlobalJniVariables(JavaVM* jvm);

/**
 * @brief Get the JNIEnv for the current thread, or nullptr if detached.
 * @return JNIEnv pointer, or nullptr if thread is not attached.
 */
JNIEnv* GetEnv();

/**
 * @brief Get the global JavaVM pointer.
 * @return The JavaVM pointer set during JNI_OnLoad.
 */
JavaVM* GetJVM();

/**
 * @brief Get a JNIEnv for the current thread, attaching if necessary.
 *        Threads attached by this function will be automatically detached
 *        when the thread exits.
 * @return A valid JNIEnv pointer for the current thread.
 */
JNIEnv* AttachCurrentThreadIfNeeded();

}  // namespace jni
}  // namespace ave

#endif  // BASE_ANDROID_JNI_JVM_H_
