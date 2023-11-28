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

import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import ohos.ace.adapter.capability.web.IAceWebErrorReceiveObject;

public class AceWebErrorReceiveObject implements IAceWebErrorReceiveObject {
    private static final String LOG_TAG = "AceWebErrorReceiveObject";

    private WebResourceError error;
    private WebResourceRequest request;

    public AceWebErrorReceiveObject(WebResourceError error, WebResourceRequest request) {
        this.error = error;
        this.request = request;
    }

    public String getRequestUrl() {
        if (this.request.getUrl() != null) {
            return this.request.getUrl().toString();
        }
        return "";
    }

    public String getErrorInfo() {
        if (this.error.getDescription() != null) {
            return this.error.getDescription().toString();
        }
        return "";
    }

    public int getErrorCode() {
        return this.error.getErrorCode();
    }
}
