/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package io.github.yoofa;

import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;

import androidx.annotation.Nullable;

import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class ThreadUtils {
    /** Utility class to be used for checking that a method is called on the correct thread. */
    public static class ThreadChecker {
        @Nullable private Thread thread = Thread.currentThread();

        /**
         * Ensures that the current thread matches the stored thread reference.
         *
         * <p>If the stored thread reference is null, it is set to the current thread. If called from a different thread than the stored one, throws an {@code IllegalStateException}.</p>
         *
         * @throws IllegalStateException if called from a thread different than the stored thread
         */
        public void checkIsOnValidThread() {
            if (thread == null) {
                thread = Thread.currentThread();
            }
            if (Thread.currentThread() != thread) {
                throw new IllegalStateException("Wrong thread");
            }
        }

        /**
         * Clears the stored thread reference, allowing future checks to bind to a new thread.
         */
        public void detachThread() {
            thread = null;
        }
    }

    /**
     * Ensures the current method is called on the Android main (UI) thread.
     *
     * @throws IllegalStateException if invoked from a thread other than the main thread
     */
    public static void checkIsOnMainThread() {
        if (Thread.currentThread() != Looper.getMainLooper().getThread()) {
            throw new IllegalStateException("Not on main thread!");
        }
    }

    /**
     * Utility interface to be used with executeUninterruptibly() to wait for blocking operations to
     * complete without getting interrupted..
     */
    public interface BlockingOperation {
        /**
 * Performs a blocking operation that may throw an InterruptedException.
 *
 * Implement this method to define an operation that can be executed uninterruptibly by utility methods.
 *
 * @throws InterruptedException if the operation is interrupted
 */
void run() throws InterruptedException;
    }

    /****
     * Executes a blocking operation to completion, ignoring interruptions until the operation finishes.
     *
     * <p>If the operation is interrupted, it is retried until it completes successfully. After completion,
     * the thread's interrupted status is restored if any interruptions occurred during execution.
     *
     * @param operation the blocking operation to execute uninterruptibly
     */
    public static void executeUninterruptibly(BlockingOperation operation) {
        boolean wasInterrupted = false;
        while (true) {
            try {
                operation.run();
                break;
            } catch (InterruptedException e) {
                // Someone is asking us to return early at our convenience. We can't cancel this
                // operation,
                // but we should preserve the information and pass it along.
                wasInterrupted = true;
            }
        }
        // Pass interruption information along.
        if (wasInterrupted) {
            Thread.currentThread().interrupt();
        }
    }

    /**
     * Attempts to join the specified thread within the given timeout, ignoring interruptions.
     *
     * If interrupted while waiting, the method recalculates the remaining time and retries until the thread terminates or the timeout expires. The current thread's interruption status is restored if any interruptions occur during the join attempts.
     *
     * @param thread the thread to join
     * @param timeoutMs the maximum time to wait in milliseconds
     * @return {@code true} if the thread is no longer alive after the join attempt; {@code false} otherwise
     */
    public static boolean joinUninterruptibly(final Thread thread, long timeoutMs) {
        final long startTimeMs = SystemClock.elapsedRealtime();
        long timeRemainingMs = timeoutMs;
        boolean wasInterrupted = false;
        while (timeRemainingMs > 0) {
            try {
                thread.join(timeRemainingMs);
                break;
            } catch (InterruptedException e) {
                // Someone is asking us to return early at our convenience. We can't cancel this
                // operation,
                // but we should preserve the information and pass it along.
                wasInterrupted = true;
                final long elapsedTimeMs = SystemClock.elapsedRealtime() - startTimeMs;
                timeRemainingMs = timeoutMs - elapsedTimeMs;
            }
        }
        // Pass interruption information along.
        if (wasInterrupted) {
            Thread.currentThread().interrupt();
        }
        return !thread.isAlive();
    }

    /**
     * Waits for the specified thread to terminate, ignoring interruptions until completion.
     *
     * This method ensures that the calling thread waits uninterruptibly for the given thread to finish execution.
     * If interrupted during the wait, the interruption is suppressed and the wait continues until the thread terminates.
     * The calling thread's interruption status is restored after the join completes.
     *
     * @param thread the thread to join
     */
    public static void joinUninterruptibly(final Thread thread) {
        executeUninterruptibly(
                new BlockingOperation() {
                    @Override
                    public void run() throws InterruptedException {
                        thread.join();
                    }
                });
    }

    /**
     * Waits for the given {@link CountDownLatch} to reach zero, ignoring interruptions until completion.
     *
     * The thread's interrupted status is restored after the wait if it was interrupted during the operation.
     *
     * @param latch the {@code CountDownLatch} to wait on
     */
    public static void awaitUninterruptibly(final CountDownLatch latch) {
        executeUninterruptibly(
                new BlockingOperation() {
                    @Override
                    public void run() throws InterruptedException {
                        latch.await();
                    }
                });
    }

    /**
     * Waits for the given {@link CountDownLatch} to reach zero within the specified timeout, ignoring interruptions.
     *
     * <p>If interrupted while waiting, the method recalculates the remaining time and continues waiting until the latch is released or the timeout expires. If interrupted at any point, the thread's interrupted status is restored before returning.
     *
     * @param barrier the {@link CountDownLatch} to wait on
     * @param timeoutMs the maximum time to wait in milliseconds
     * @return {@code true} if the latch reached zero before the timeout, {@code false} otherwise
     */
    public static boolean awaitUninterruptibly(CountDownLatch barrier, long timeoutMs) {
        final long startTimeMs = SystemClock.elapsedRealtime();
        long timeRemainingMs = timeoutMs;
        boolean wasInterrupted = false;
        boolean result = false;
        do {
            try {
                result = barrier.await(timeRemainingMs, TimeUnit.MILLISECONDS);
                break;
            } catch (InterruptedException e) {
                // Someone is asking us to return early at our convenience. We can't cancel this
                // operation,
                // but we should preserve the information and pass it along.
                wasInterrupted = true;
                final long elapsedTimeMs = SystemClock.elapsedRealtime() - startTimeMs;
                timeRemainingMs = timeoutMs - elapsedTimeMs;
            }
        } while (timeRemainingMs > 0);
        // Pass interruption information along.
        if (wasInterrupted) {
            Thread.currentThread().interrupt();
        }
        return result;
    }

    /**
     * Executes the given {@code callable} on the specified handler's thread and waits uninterruptibly for its result.
     *
     * <p>If called from the handler's thread, the callable is executed directly. Otherwise, the callable is posted to the handler's message queue, and this method blocks until execution completes. Any exception thrown by the callable is rethrown as a {@code RuntimeException} with a combined stack trace from both threads.
     *
     * @param handler the handler whose thread will execute the callable
     * @param callable the operation to execute
     * @return the result returned by the callable
     * @throws RuntimeException if the callable throws an exception
     */
    public static <V> V invokeAtFrontUninterruptibly(
            final Handler handler, final Callable<V> callable) {
        if (handler.getLooper().getThread() == Thread.currentThread()) {
            try {
                return callable.call();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
        // Place-holder classes that are assignable inside nested class.
        class CaughtException {
            Exception e;
        }
        class Result {
            public V value;
        }
        final Result result = new Result();
        final CaughtException caughtException = new CaughtException();
        final CountDownLatch barrier = new CountDownLatch(1);
        handler.post(
                new Runnable() {
                    /**
                     * Executes the provided callable, storing its result or any thrown exception, and signals completion via the latch.
                     */
                    @Override
                    public void run() {
                        try {
                            result.value = callable.call();
                        } catch (Exception e) {
                            caughtException.e = e;
                        }
                        barrier.countDown();
                    }
                });
        awaitUninterruptibly(barrier);
        // Re-throw any runtime exception caught inside the other thread. Since this is an invoke,
        // add
        // stack trace for the waiting thread as well.
        if (caughtException.e != null) {
            final RuntimeException runtimeException = new RuntimeException(caughtException.e);
            runtimeException.setStackTrace(
                    concatStackTraces(
                            caughtException.e.getStackTrace(), runtimeException.getStackTrace()));
            throw runtimeException;
        }
        return result.value;
    }

    /**
     * Executes the given {@code Runnable} on the specified {@code Handler}'s thread, posting it at the front of the message queue and waiting uninterruptibly for its completion.
     *
     * <p>If called from the handler's thread, the runnable is executed immediately. Otherwise, the runnable is posted and this method blocks until it finishes, ignoring interruptions.
     *
     * @param handler the handler whose thread will execute the runnable
     * @param runner the runnable to execute
     */
    public static void invokeAtFrontUninterruptibly(final Handler handler, final Runnable runner) {
        invokeAtFrontUninterruptibly(
                handler,
                new Callable<Void>() {
                    /****
                     * Executes the provided {@code Runnable} and returns {@code null}.
                     *
                     * Used to adapt a {@code Runnable} to a {@code Callable<Void>} interface.
                     *
                     * @return always {@code null}
                     */
                    @Override
                    public Void call() {
                        runner.run();
                        return null;
                    }
                });
    }

    /**
     * Combines two arrays of stack trace elements into a single array.
     *
     * @param inner the first stack trace array to include
     * @param outer the second stack trace array to append after the first
     * @return a new array containing all elements from {@code inner} followed by all elements from {@code outer}
     */
    static StackTraceElement[] concatStackTraces(
            StackTraceElement[] inner, StackTraceElement[] outer) {
        final StackTraceElement[] combined = new StackTraceElement[inner.length + outer.length];
        System.arraycopy(inner, 0, combined, 0, inner.length);
        System.arraycopy(outer, 0, combined, inner.length, outer.length);
        return combined;
    }
}
