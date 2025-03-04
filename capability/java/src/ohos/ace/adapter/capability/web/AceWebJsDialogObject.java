/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.web;

import android.webkit.JsResult;
import ohos.ace.adapter.ALog;

/**
 * AceWebJsDialogObject is used to handle the dialog from the web page.
 *
 * @since 2024-05-31
 */
public class AceWebJsDialogObject {
    private static final String LOG_TAG = "AceWebJsDialogObject";

    private String url;
    private String message;
    private JsResult result;

    /**
     * Constructor of AceWebJsDialogObject.
     *
     * @param url the url of the web page
     * @param message the message of the dialog
     * @param result the JsResult object
     */
    public AceWebJsDialogObject(String url, String message, JsResult result) {
        this.url = url;
        this.message = message;
        this.result = result;
    }

    /**
     * Get the url of the web page.
     *
     * @return the url of the web page
     */
    public String getUrl() {
        return this.url;
    }

    /**
     * Get the message of the dialog.
     *
     * @return the message of the dialog
     */
    public String getMessage() {
        return this.message;
    }

    /**
     * Confirm the dialog.
     */
    public void confirm() {
        try {
            this.result.confirm();
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call confirm method failed");
        }
    }

    /**
     * Cancel the dialog.
     */
    public void cancel() {
        try {
            this.result.cancel();
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call cancel method failed");
        }
    }
}
