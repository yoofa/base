/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package io.github.yoofa;

import android.content.Context;

/**
 * Class for storing the application context and retrieving it in a static context. Similar to
 * org.chromium.base.ContextUtils.
 */
public class ContextUtils {
    private static final String TAG = "ContextUtils";
    private static Context applicationContext;

    /**
     * Sets the static application context for later retrieval.
     *
     * <p>This method must be called with a non-null application context before creating a
     * PeerConnectionFactory. The stored context should not be changed after initialization.
     *
     * @param applicationContext the non-null Android application context to store
     * @throws IllegalArgumentException if {@code applicationContext} is null
     */
    public static void initialize(Context applicationContext) {
        if (applicationContext == null) {
            throw new IllegalArgumentException(
                    "Application context cannot be null for ContextUtils.initialize.");
        }
        ContextUtils.applicationContext = applicationContext;
    }

    /**
     * Retrieves the stored application context.
     *
     * @return the application context previously set via {@link #initialize(Context)}
     * @deprecated Use alternative context management as referenced in crbug.com/webrtc/8937.
     */
    @Deprecated
    public static Context getApplicationContext() {
        return applicationContext;
    }
}
