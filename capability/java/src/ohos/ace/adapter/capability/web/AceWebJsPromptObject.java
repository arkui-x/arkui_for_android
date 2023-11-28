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

import android.webkit.JsPromptResult;
import ohos.ace.adapter.ALog;

public class AceWebJsPromptObject {
    private static final String LOG_TAG = "AceWebJsPromptObject";

    private String url;
    private String message;
    private String defaultValue;
    private JsPromptResult result;

    public AceWebJsPromptObject(String url, String message, String defaultValue, JsPromptResult result) {
        this.url = url;
        this.message = message;
        this.defaultValue = defaultValue;
        this.result = result;
    }

    public String getUrl() {
        return this.url;
    }

    public String getMessage() {
        return this.message;
    }

    public String getDefaultValue() {
        return this.defaultValue;
    }

    public void cancel() {
        try {
            this.result.cancel();
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call cancel method failed");
        }
    }

    public void confirm() {
        try {
            this.result.confirm();
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call confirm method failed");
        }
    }

    public void confirm(String result) {
        try {
            this.result.confirm(result);
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call confirm method failed");
        }
    }
}
