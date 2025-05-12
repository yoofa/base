/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package io.github.yoofa;

import io.github.yoofa.Logging.Severity;

/**
 * Java interface for WebRTC logging. The default implementation uses webrtc.Logging.
 *
 * <p>When injected, the Loggable will receive logging from both Java and native.
 */
public interface Loggable {
    /****
 * Receives a log message with its severity level and associated tag.
 *
 * @param message  the log message content
 * @param severity the severity level of the log message
 * @param tag      a tag identifying the source or category of the log message
 */
public void onLogMessage(String message, Severity severity, String tag);
}
