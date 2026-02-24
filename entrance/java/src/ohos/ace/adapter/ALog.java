/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ohos.ace.adapter;

import android.util.Log;

import java.util.concurrent.locks.ReentrantLock;

/**
 * Log Wrapper for specific platform
 *
 * @since 1
 */
public class ALog {
    private static ILogger logger;
    private static final ReentrantLock loggerLock = new ReentrantLock();
    private static int logLevel = 0;

    private ALog() {
        /* Do nothing */
    }

    /**
     * Get the Logger.
     *
     * @return the Logger.
     */
    public static ILogger getLogger() {
        return logger;
    }

    /**
     * Get the Logger level.
     *
     * @return the Logger level.
     */
    public static int getLoggerLevel() {
        return logLevel;
    }

    /**
     * Sets the Logger instance.
     *
     * @param log logger instance
     */
    public static void setLogger(ILogger log) {
        if (log == null) {
            return;
        }
        loggerLock.lock();
        logger = log;
        loggerLock.unlock();
    }

    /**
     * Set the Logger level.
     *
     * @param level log level
     */
    public static void setLoggerLevel(int level) {
        logLevel = level;
    }

    /**
     * Determines if debug log is permitted output.
     *
     * @return true if debuggable, false otherwise.
     */
    public static boolean isDebuggable() {
        if (logger == null) {
            return false;
        }
        return logger.isDebuggable();
    }

    /**
     * Log wrapper for print debug log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    public static void d(String tag, String msg) {
        if (logger != null && logLevel <= ILogger.LOG_DEBUG) {
            logger.d(tag, msg);
        } else {
            Log.d(tag, msg);
        }
    }

    /**
     * Log wrapper for print info log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    public static void i(String tag, String msg) {
        if (logger != null && logLevel <= ILogger.LOG_INFO) {
            logger.i(tag, msg);
        } else {
            Log.i(tag, msg);
        }
    }

    /**
     * Log wrapper for print warning log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    public static void w(String tag, String msg) {
        if (logger != null && logLevel <= ILogger.LOG_WARN) {
            logger.w(tag, msg);
        } else {
            Log.w(tag, msg);
        }
    }

    /**
     * Log wrapper for print error log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    public static void e(String tag, String msg) {
        if (logger != null && logLevel <= ILogger.LOG_ERROR) {
            logger.e(tag, msg);
        } else {
            Log.e(tag, msg);
        }
    }

    /**
     * Log wrapper for report jank log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    public static void jankLog(int tag, String msg) {
        if (logger == null) {
            return;
        }
        logger.jankLog(tag, msg);
    }
}
