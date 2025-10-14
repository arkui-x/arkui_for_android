/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

/**
 * Represents an object that extends AceWebResourceRequestObject to handle web
 * resource loading errors.
 * This class implements IAceWebErrorReceiveObject and is designed to manage and
 * manipulate web resource requests and their corresponding error states.
 *
 * @since 2023-12-13
 */
public class AceWebErrorReceiveObject extends AceWebResourceRequestObject implements IAceWebErrorReceiveObject {
    private static final String LOG_TAG = "AceWebErrorReceiveObject";

    private WebResourceError error;

    private int errorCode = 0;

    private String errorInfo = "";

    /**
     * Constructs an AceWebErrorReceiveObject with the specified WebResourceError
     * and WebResourceRequest.
     *
     * @param errorCode   the error code representing the type of web error encountered
     * @param errorInfo   a descriptive message providing details about the error
     * @param request     the {@link WebResourceRequest} associated with the error event
     */
    public AceWebErrorReceiveObject(int errorCode, String errorInfo, WebResourceRequest request) {
        super(request);
        this.errorCode = errorCode;
        this.errorInfo = errorInfo;
    }

    /**
     * Get error info
     *
     * @return error info
     */
    @Override
    public String getErrorInfo() {
        return this.errorInfo;
    }

    /**
     * Get request url
     *
     * @return request url
     */
    @Override
    public int getErrorCode() {
        return this.errorCode;
    }
}
