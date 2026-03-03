/*
 *  Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 *  Distributed under terms of the GPLv2 license.
 */

package io.github.yoofa;

import org.jni_zero.CalledByNative;

/**
 * Helper class to provide the application ClassLoader for JNI.
 * Called from native code to obtain a ClassLoader that can find app classes
 * from any thread (including native-created threads).
 */
public class AveClassLoader {
    @CalledByNative
    static ClassLoader getClassLoader() {
        return AveClassLoader.class.getClassLoader();
    }
}
