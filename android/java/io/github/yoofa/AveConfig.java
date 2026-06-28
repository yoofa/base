/*
 * Copyright (C) 2026 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

package io.github.yoofa;

import org.jni_zero.JniType;
import org.jni_zero.NativeMethods;

/** Process-wide AVE configuration. Set options before creating players or codecs. */
public final class AveConfig {
    private AveConfig() {}

    /** Enables codec-specific diagnostics, including the Android video input dump. */
    public static void setCodecDebugEnabled(boolean enabled) {
        AveConfigJni.get().setCodecDebugEnabled(enabled);
    }

    /** Enables verbose per-frame diagnostics in all demuxer implementations. */
    public static void setDemuxerLogsEnabled(boolean enabled) {
        AveConfigJni.get().setDemuxerLogsEnabled(enabled);
    }

    @NativeMethods
    interface Natives {
        void setCodecDebugEnabled(@JniType("bool") boolean enabled);

        void setDemuxerLogsEnabled(@JniType("bool") boolean enabled);
    }
}
