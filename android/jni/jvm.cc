/*
 * jvm.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/android/jni/jvm.h"

#include <jni.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cstdio>
#include <string>

#include "base/checks.h"
#include "base/logging.h"
#include "third_party/jni_zero/jni_zero.h"

namespace ave {
namespace jni {

static JavaVM* g_jvm = nullptr;

static pthread_once_t g_jni_ptr_once = PTHREAD_ONCE_INIT;

// Per-thread JNIEnv* storage. Non-NULL only in threads we attached ourselves.
static pthread_key_t g_jni_ptr;

JavaVM* GetJVM() {
  AVE_DCHECK(g_jvm) << "JNI_OnLoad failed to run?";
  return g_jvm;
}

JNIEnv* GetEnv() {
  void* env = nullptr;
  jint status = g_jvm->GetEnv(&env, JNI_VERSION_1_6);
  AVE_DCHECK(((env != nullptr) && (status == JNI_OK)) ||
             ((env == nullptr) && (status == JNI_EDETACHED)))
      << "Unexpected GetEnv return: " << status << ":" << env;
  return reinterpret_cast<JNIEnv*>(env);
}

static void ThreadDestructor(void* prev_jni_ptr) {
  if (!GetEnv()) {
    return;
  }
  AVE_DCHECK(GetEnv() == prev_jni_ptr)
      << "Detaching from another thread: " << prev_jni_ptr << ":"
      << GetEnv();
  jint status = g_jvm->DetachCurrentThread();
  AVE_DCHECK(status == JNI_OK) << "Failed to detach thread: " << status;
}

static void CreateJNIPtrKey() {
  AVE_CHECK(!pthread_key_create(&g_jni_ptr, &ThreadDestructor))
      << "pthread_key_create";
}

static void HandleException(JNIEnv* env) {
  env->ExceptionDescribe();
  env->ExceptionClear();
  AVE_LOG(LS_ERROR) << "Java exception occurred in JNI code";
}

jint InitGlobalJniVariables(JavaVM* jvm) {
  AVE_CHECK(!g_jvm) << "InitGlobalJniVariables called twice!";
  g_jvm = jvm;
  AVE_CHECK(g_jvm) << "InitGlobalJniVariables handed NULL?";

  jni_zero::SetExceptionHandler(&HandleException);
  jni_zero::InitVM(jvm);

  AVE_CHECK(!pthread_once(&g_jni_ptr_once, &CreateJNIPtrKey))
      << "pthread_once";

  JNIEnv* jni = nullptr;
  if (jvm->GetEnv(reinterpret_cast<void**>(&jni), JNI_VERSION_1_6) != JNI_OK) {
    return -1;
  }

  return JNI_VERSION_1_6;
}

static std::string GetThreadId() {
  char buf[21];
  snprintf(buf, sizeof(buf), "%ld", static_cast<long>(syscall(__NR_gettid)));
  return std::string(buf);
}

static std::string GetThreadName() {
  char name[17] = {0};
  if (prctl(PR_GET_NAME, name) != 0) {
    return std::string("<noname>");
  }
  return std::string(name);
}

JNIEnv* AttachCurrentThreadIfNeeded() {
  JNIEnv* jni = GetEnv();
  if (jni) {
    return jni;
  }
  AVE_DCHECK(!pthread_getspecific(g_jni_ptr))
      << "TLS has a JNIEnv* but not attached?";

  std::string name(GetThreadName() + " - " + GetThreadId());
  JavaVMAttachArgs args;
  args.version = JNI_VERSION_1_6;
  args.name = &name[0];
  args.group = nullptr;

  JNIEnv* env = nullptr;
  AVE_CHECK(!g_jvm->AttachCurrentThread(&env, &args))
      << "Failed to attach thread";
  AVE_CHECK(env) << "AttachCurrentThread handed back NULL!";
  jni = env;
  AVE_CHECK(!pthread_setspecific(g_jni_ptr, jni)) << "pthread_setspecific";
  return jni;
}

}  // namespace jni
}  // namespace ave
