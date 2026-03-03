/*
 * class_loader.cc
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/android/jni/class_loader.h"

#include <jni.h>

#include <algorithm>
#include <string>

#include "base/checks.h"
#include "base/logging.h"
#include "jni_headers/base/android/generated_base_jni/AveClassLoader_jni.h"
#include "third_party/jni_zero/jni_zero.h"

namespace ave {
namespace jni {

namespace {

class ClassLoader {
 public:
  explicit ClassLoader(JNIEnv* env)
      : class_loader_(Java_AveClassLoader_getClassLoader(env)) {
    class_loader_class_ = reinterpret_cast<jclass>(
        env->NewGlobalRef(env->FindClass("java/lang/ClassLoader")));
    AVE_CHECK(!env->ExceptionCheck());
    load_class_method_ =
        env->GetMethodID(class_loader_class_, "loadClass",
                         "(Ljava/lang/String;)Ljava/lang/Class;");
    AVE_CHECK(!env->ExceptionCheck());
  }

  jni_zero::ScopedJavaLocalRef<jclass> FindClass(JNIEnv* env,
                                                  const char* c_name) {
    // ClassLoader.loadClass expects dots instead of slashes.
    std::string name(c_name);
    std::replace(name.begin(), name.end(), '/', '.');
    jstring j_name = env->NewStringUTF(name.c_str());
    const jclass clazz = static_cast<jclass>(
        env->CallObjectMethod(class_loader_.obj(), load_class_method_, j_name));
    env->DeleteLocalRef(j_name);
    AVE_CHECK(!env->ExceptionCheck())
        << "Failed to find class: " << c_name;
    return jni_zero::ScopedJavaLocalRef<jclass>::Adopt(env, clazz);
  }

 private:
  jni_zero::ScopedJavaGlobalRef<jobject> class_loader_;
  jclass class_loader_class_;
  jmethodID load_class_method_;
};

static ClassLoader* g_class_loader = nullptr;

jclass ResolveClass(JNIEnv* env,
                    const char* class_name,
                    const char* /*unused*/) {
  AVE_CHECK(g_class_loader);
  return static_cast<jclass>(
      g_class_loader->FindClass(env, class_name).Release());
}

}  // namespace

void InitClassLoader(JNIEnv* env) {
  AVE_CHECK(g_class_loader == nullptr);
  g_class_loader = new ClassLoader(env);
  jni_zero::SetClassResolver(&ResolveClass);
}

jni_zero::ScopedJavaLocalRef<jclass> GetClass(JNIEnv* env,
                                               const char* c_name) {
  if (g_class_loader != nullptr) {
    return g_class_loader->FindClass(env, c_name);
  }
  // Fallback: convert dots to slashes for JNI's FindClass.
  std::string name(c_name);
  std::replace(name.begin(), name.end(), '.', '/');
  return jni_zero::ScopedJavaLocalRef<jclass>::Adopt(
      env, env->FindClass(name.c_str()));
}

}  // namespace jni
}  // namespace ave
