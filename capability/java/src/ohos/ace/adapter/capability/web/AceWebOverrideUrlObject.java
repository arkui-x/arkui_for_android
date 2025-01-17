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

import android.webkit.WebResourceRequest;

/**
 * Represents an object that extends AceWebResourceRequestObject to handle URL
 * override functionality.
 * This class is designed to manage and manipulate web resource requests with
 * additional features.
 *
 * @since 2024-5-31
 */
public class AceWebOverrideUrlObject extends AceWebResourceRequestObject {
    private static final String LOG_TAG = "AceWebOverrideUrlObject";

    /**
     * Constructs an AceWebOverrideUrlObject with the specified WebResourceRequest.
     *
     * @param request The WebResourceRequest associated with this object.
     */
    public AceWebOverrideUrlObject(WebResourceRequest request) {
        super(request);
    }
}
