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

import android.webkit.WebResourceResponse;
import android.webkit.WebResourceRequest;

public class AceWebHttpErrorReceiveObject {
    private static final String LOG_TAG = "AceWebHttpErrorReceiveObject";

    private WebResourceRequest request;
    private WebResourceResponse response;

    public AceWebHttpErrorReceiveObject(WebResourceRequest request, WebResourceResponse response) {
        this.request = request;
        this.response = response;
    }

    public String getRequestUrl() {
        if (this.request.getUrl() != null) {
            return this.request.getUrl().toString();
        }
        return "";
    }

    /**
     * Retrieves the MIME type of the response.
     *
     * @return A string representing the MIME type, or an empty string if the
     */
    public String getResponseMimeType() {
        if (this.response.getMimeType() != null) {
            return this.response.getMimeType().toString();
        }
        return "";
    }

    /**
     * Retrieves the encoding of the response.
     *
     * @return A string representing the encoding, or an empty string if the encoding is not available.
     */
    public String getResponseEncoding() {
        if (this.response.getEncoding() != null) {
            return this.response.getEncoding().toString();
        }
        return "";
    }

    /**
     * Retrieves the status code of the response.
     *
     * @return An integer representing the status code, or -1 if the status code is not available.
     */
    public int getResponseCode() {
        return this.response.getStatusCode();
    }
}
