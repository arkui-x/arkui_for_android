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

/**
 * Common interface for logging
 *
 * @since 1
 */
public interface ILogger {
    /**
     * Log level of debug.
     */
    public static final int LOG_DEBUG = 0;

    /**
     * Log level of info.
     */
    public static final int LOG_INFO = 1;

    /**
     * Log level of warning.
     */
    public static final int LOG_WARN = 2;

    /**
     * Log level of error.
     */
    public static final int LOG_ERROR = 3;

    /**
     * Log level of fatal.
     */
    public static final int LOG_FATAL = 4;

    /**
     * Determines if debug log is permitted output.
     *
     * @return true if debuggable, false otherwise.
     */
    boolean isDebuggable();

    /**
     * Log wrapper for print debug log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    void d(String tag, String msg);

    /**
     * Log wrapper for print info log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    void i(String tag, String msg);

    /**
     * Log wrapper for print warning log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    void w(String tag, String msg);

    /**
     * Log wrapper for print error log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    void e(String tag, String msg);

    /**
     * Log wrapper for print fatal log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    void f(String tag, String msg);

    /**
     * Log wrapper for report jank log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    void jankLog(int tag, String msg);
}
