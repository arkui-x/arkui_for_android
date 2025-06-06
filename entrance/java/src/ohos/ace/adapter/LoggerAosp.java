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

/**
 * Logger implementation in Aosp platform
 *
 * @since 1
 */
public class LoggerAosp implements ILogger {
    /**
     * Determines if debug log is permitted output.
     *
     * @return true if debuggable, false otherwise.
     */
    @Override
    public boolean isDebuggable() {
        return false;
    }

    /**
     * Log wrapper for print debug log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    @Override
    public void d(String tag, String msg) {
        Log.d(tag, msg);
    }

    /**
     * Log wrapper for print info log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    @Override
    public void i(String tag, String msg) {
        Log.i(tag, msg);
    }

    /**
     * Log wrapper for print warning log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    @Override
    public void w(String tag, String msg) {
        Log.w(tag, msg);
    }

    /**
     * Log wrapper for print error log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    @Override
    public void e(String tag, String msg) {
        Log.e(tag, msg);
    }

    /**
     * Log wrapper for print fatal log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    @Override
    public void f(String tag, String msg) {}

    /**
     * Log wrapper for report jank log.
     *
     * @param tag message tag
     * @param msg message to print
     */
    @Override
    public void jankLog(int tag, String msg) {
    }
}
