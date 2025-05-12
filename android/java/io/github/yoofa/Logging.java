/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package io.github.yoofa;

import androidx.annotation.Nullable;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.EnumSet;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Java wrapper for WebRTC logging. Logging defaults to java.util.logging.Logger, but a custom
 * logger implementing the Loggable interface can be injected along with a Severity. All subsequent
 * log messages will then be redirected to the injected Loggable, except those with a severity lower
 * than the specified severity, which will be discarded.
 *
 * <p>It is also possible to switch to native logging (rtc::LogMessage) if one of the following
 * static functions are called from the app: - Logging.enableLogThreads -
 * Logging.enableLogTimeStamps - Logging.enableLogToDebugOutput
 *
 * <p>The priority goes: 1. Injected loggable 2. Native logging 3. Fallback logging. Only one method
 * will be used at a time.
 *
 * <p>Injecting a Loggable or using any of the enable... methods requires that the native library is
 * loaded, using PeerConnectionFactory.initialize.
 */
public class Logging {
    private static final Logger fallbackLogger = createFallbackLogger();
    private static volatile boolean loggingEnabled;
    @Nullable private static Loggable loggable;
    private static Severity loggableSeverity;

    /**
     * Creates and configures a fallback logger for use when no other logging mechanism is available.
     *
     * @return a {@link Logger} instance named "io.github.yoofa.Logging" with logging level set to {@code Level.ALL}
     */
    private static Logger createFallbackLogger() {
        final Logger fallbackLogger = Logger.getLogger("io.github.yoofa.Logging");
        fallbackLogger.setLevel(Level.ALL);
        return fallbackLogger;
    }

    /**
     * Injects a custom logger to handle log messages at or above the specified severity.
     *
     * @param injectedLoggable the custom logger to use for logging, or null to leave the current logger unchanged
     * @param severity the minimum severity level for messages to be forwarded to the injected logger
     */
    static void injectLoggable(Loggable injectedLoggable, Severity severity) {
        if (injectedLoggable != null) {
            loggable = injectedLoggable;
            loggableSeverity = severity;
        }
    }

    /**
     * Removes the currently injected custom logger, if any, reverting to native or fallback logging.
     */
    static void deleteInjectedLoggable() {
        loggable = null;
    }

    // TODO(solenberg): Remove once dependent projects updated.
    @Deprecated
    public enum TraceLevel {
        TRACE_NONE(0x0000),
        TRACE_STATEINFO(0x0001),
        TRACE_WARNING(0x0002),
        TRACE_ERROR(0x0004),
        TRACE_CRITICAL(0x0008),
        TRACE_APICALL(0x0010),
        TRACE_DEFAULT(0x00ff),
        TRACE_MODULECALL(0x0020),
        TRACE_MEMORY(0x0100),
        TRACE_TIMER(0x0200),
        TRACE_STREAM(0x0400),
        TRACE_DEBUG(0x0800),
        TRACE_INFO(0x1000),
        TRACE_TERSEINFO(0x2000),
        TRACE_ALL(0xffff);

        public final int level;

        /**
         * Constructs a TraceLevel enum constant with the specified bit flag value.
         *
         * @param level the bit flag representing the trace level
         */
        TraceLevel(int level) {
            this.level = level;
        }
    }

    // Keep in sync with webrtc/rtc_base/logging.h:LoggingSeverity.
    public enum Severity {
        LS_VERBOSE,
        LS_INFO,
        LS_WARNING,
        LS_ERROR,
        LS_NONE
    }

    /**
     * Enables logging of thread information in native logging output.
     */
    public static void enableLogThreads() {
        nativeEnableLogThreads();
    }

    /****
     * Enables the inclusion of timestamps in native log output.
     */
    public static void enableLogTimeStamps() {
        nativeEnableLogTimeStamps();
    }

    /****
     * Deprecated stub for enabling tracing; does nothing.
     *
     * @deprecated This method is no longer functional and will be removed in a future release.
     */
    @Deprecated
    public static void enableTracing(String path, EnumSet<TraceLevel> levels) {}

    // Enable diagnostic logging for messages of `severity` to the platform debug
    // output. On Android, the output will be directed to Logcat.
    // Note: this function starts collecting the output of the RTC_LOG() macros.
    /**
     * Enables native logging to the platform's debug output at the specified severity level.
     *
     * @param severity the minimum severity level for messages to be logged to native debug output
     * @throws IllegalStateException if a custom Loggable is currently injected
     */
    @SuppressWarnings({"EnumOrdinal", "NoSynchronizedMethodCheck"})
    public static synchronized void enableLogToDebugOutput(Severity severity) {
        if (loggable != null) {
            throw new IllegalStateException(
                    "Logging to native debug output not supported while Loggable is injected. "
                            + "Delete the Loggable before calling this method.");
        }
        nativeEnableLogToDebugOutput(severity.ordinal());
        loggingEnabled = true;
    }

    /**
     * Logs a message with the specified severity, tag, and content using the highest-priority available logging mechanism.
     *
     * <p>
     * If a custom {@code Loggable} is injected and the message severity meets or exceeds its configured threshold,
     * the message is forwarded to the injected logger. Otherwise, if native logging is enabled, the message is logged
     * via the native WebRTC logging system. If neither is available, the message is logged using the fallback Java logger.
     * </p>
     *
     * @param severity the severity level of the log message
     * @param tag a non-null tag identifying the log source
     * @param message a non-null message to log
     * @throws IllegalArgumentException if {@code tag} or {@code message} is null
     */
    @SuppressWarnings("EnumOrdinal")
    public static void log(Severity severity, String tag, String message) {
        if (tag == null || message == null) {
            throw new IllegalArgumentException("Logging tag or message may not be null.");
        }
        if (loggable != null) {
            // Filter log messages below loggableSeverity.
            if (severity.ordinal() < loggableSeverity.ordinal()) {
                return;
            }
            loggable.onLogMessage(message, severity, tag);
            return;
        }

        // Try native logging if no loggable is injected.
        if (loggingEnabled) {
            nativeLog(severity.ordinal(), tag, message);
            return;
        }

        // Fallback to system log.
        Level level;
        switch (severity) {
            case LS_ERROR:
                level = Level.SEVERE;
                break;
            case LS_WARNING:
                level = Level.WARNING;
                break;
            case LS_INFO:
                level = Level.INFO;
                break;
            default:
                level = Level.FINE;
                break;
        }
        fallbackLogger.log(level, tag + ": " + message);
    }

    /**
     * Logs an informational message with the specified tag.
     *
     * Equivalent to logging with severity level {@code LS_INFO}.
     *
     * @param tag the tag identifying the source of the log message
     * @param message the message to log
     */
    public static void d(String tag, String message) {
        log(Severity.LS_INFO, tag, message);
    }

    /**
     * Logs an error-level message with the specified tag.
     *
     * @param tag the tag identifying the source of the log message
     * @param message the message to log
     */
    public static void e(String tag, String message) {
        log(Severity.LS_ERROR, tag, message);
    }

    /**
     * Logs a warning message with the specified tag.
     *
     * @param tag the tag identifying the source of the log message
     * @param message the warning message to log
     */
    public static void w(String tag, String message) {
        log(Severity.LS_WARNING, tag, message);
    }

    /****
     * Logs an error message along with the string representation and stack trace of a throwable at error severity.
     *
     * @param tag the tag identifying the source of the log message
     * @param message the error message to log
     * @param e the throwable whose details and stack trace will be logged
     */
    public static void e(String tag, String message, Throwable e) {
        log(Severity.LS_ERROR, tag, message);
        log(Severity.LS_ERROR, tag, e.toString());
        log(Severity.LS_ERROR, tag, getStackTraceString(e));
    }

    /**
     * Logs a warning message along with a throwable's description and stack trace.
     *
     * @param tag the tag identifying the log source
     * @param message the warning message to log
     * @param e the throwable whose details and stack trace will be logged
     */
    public static void w(String tag, String message, Throwable e) {
        log(Severity.LS_WARNING, tag, message);
        log(Severity.LS_WARNING, tag, e.toString());
        log(Severity.LS_WARNING, tag, getStackTraceString(e));
    }

    /**
     * Logs a message at verbose severity.
     *
     * @param tag the tag identifying the source of the log message
     * @param message the message to log
     */
    public static void v(String tag, String message) {
        log(Severity.LS_VERBOSE, tag, message);
    }

    /**
     * Returns the stack trace of a throwable as a string.
     *
     * @param e the throwable whose stack trace is to be converted; if null, returns an empty string
     * @return the stack trace as a string, or an empty string if the throwable is null
     */
    private static String getStackTraceString(Throwable e) {
        if (e == null) {
            return "";
        }

        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        e.printStackTrace(pw);
        return sw.toString();
    }

    /****
 * Enables native logging to the platform's debug output at the specified severity level.
 *
 * @param nativeSeverity the native severity level for logging output
 */
private static native void nativeEnableLogToDebugOutput(int nativeSeverity);

    /****
 * Enables native logging of thread information for WebRTC logs.
 */
private static native void nativeEnableLogThreads();

    /****
 * Enables native logging of timestamps for log messages.
 */
private static native void nativeEnableLogTimeStamps();

    /****
 * Logs a message to the native WebRTC logging system with the specified severity and tag.
 *
 * @param severity the native severity level for the log message
 * @param tag the tag identifying the log source
 * @param message the log message to record
 */
private static native void nativeLog(int severity, String tag, String message);
}
