/*
 * Copyright (C) 2026 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

package io.github.yoofa;

import org.jni_zero.NativeMethods;

/** Process-wide AVE configuration. Set options before creating players or codecs. */
public final class AveConfig {
    private AveConfig() {}

    /** Enables codec-specific diagnostics, including the Android video input dump. */
    public static void setCodecDebugEnabled(boolean enabled) {
        AveConfigJni.get().setCodecDebugEnabled(enabled);
    }

    @NativeMethods
    interface Natives {
        void setCodecDebugEnabled(boolean enabled);
    }
}
