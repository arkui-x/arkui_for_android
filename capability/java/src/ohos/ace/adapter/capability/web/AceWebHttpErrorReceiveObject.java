/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
import java.util.HashMap;
import java.util.Map;

/**
 * Represents an object that extends AceWebResourceRequestObject to handle HTTP
 * error responses.
 * This class is designed to manage and manipulate web resource requests and
 * their corresponding error responses.
 *
 * @since 2024-5-31
 */
public class AceWebHttpErrorReceiveObject extends AceWebResourceRequestObject {
    private static final String LOG_TAG = "AceWebHttpErrorReceiveObject";

    private WebResourceResponse response;

    /**
     * Constructs an AceWebHttpErrorReceiveObject with the specified
     * WebResourceRequest and WebResourceResponse.
     *
     * @param request  The WebResourceRequest associated with this object.
     * @param response The WebResourceResponse associated with this object,
     *                 representing the error response.
     */
    public AceWebHttpErrorReceiveObject(WebResourceRequest request, WebResourceResponse response) {
        super(request);
        this.response = response;
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

    /**
     * Retrieves the data of the response.
     *
     * @return A string representing the response data, or an empty string if the
     *         data is not available.
     */
    public String getResponseData() {
        if (this.response.getData() != null) {
            return this.response.getData().toString();
        }
        return "";
    }

    /**
     * Retrieves the reason phrase of the response.
     *
     * @return A string representing the reason phrase, or an empty string if the
     *         reason phrase is not available.
     */
    public String getReason() {
        if (this.response.getReasonPhrase() != null) {
            return this.response.getReasonPhrase();
        }
        return "";
    }

    /**
     * Retrieves the response headers as a map.
     *
     * @return A map of response headers, or an empty map if the headers are not
     *         available.
     */
    public Map<String, String> getResponseHeader() {
        if (this.response.getResponseHeaders() != null) {
            return this.response.getResponseHeaders();
        }
        return new HashMap<String, String>();
    }
}
