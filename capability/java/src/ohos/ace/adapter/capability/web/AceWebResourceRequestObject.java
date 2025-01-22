/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import android.webkit.WebResourceRequest;
import java.util.HashMap;
import java.util.Map;

/**
 * Represents a web resource request object for handling web resource requests.
 *
 * @since 2025-1-14
 */
public class AceWebResourceRequestObject {
    private WebResourceRequest request;

    /**
     * Constructs an AceWebResourceRequestObject with the specified
     * WebResourceRequest.
     *
     * @param request The WebResourceRequest associated with this object.
     */
    public AceWebResourceRequestObject(WebResourceRequest request) {
        this.request = request;
    }

    /**
     * Retrieves the URL of the request.
     *
     * @return string representing the URL of the request. If the URL is null, an
     *         empty string is returned.
     */
    public String getRequestUrl() {
        if (this.request.getUrl() != null) {
            return this.request.getUrl().toString();
        }
        return "";
    }

    /**
     * Retrieves the HTTP method of the request.
     *
     * @return A string representing the HTTP method, or an empty string if the
     *         method is not available.
     */
    public String getMethod() {
        if (this.request.getMethod() != null) {
            return this.request.getMethod().toString();
        }
        return "";
    }

    /**
     * Checks if the request has a gesture.
     *
     * @return true if the request has a gesture, false otherwise.
     */
    public boolean isRequestGesture() {
        return this.request.hasGesture();
    }

    /**
     * Checks if the request is for the main frame.
     *
     * @return true if the request is for the main frame, false otherwise.
     */
    public boolean isMainFrame() {
        return this.request.isForMainFrame();
    }

    /**
     * Checks if the request is a redirect.
     *
     * @return true if the request is a redirect, false otherwise.
     */
    public boolean isRedirect() {
        return this.request.isRedirect();
    }

    /**
     * Retrieves the request headers as a map.
     *
     * @return A map of request headers, or an empty map if the headers are not
     *         available.
     */
    public Map<String, String> getRequestHeader() {
        if (this.request.getRequestHeaders() != null) {
            return this.request.getRequestHeaders();
        }
        return new HashMap<String, String>();
    }
}
