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

import android.webkit.WebResourceRequest;

/**
 * AceWebOverrideUrlObject class is used to override the url
 *
 * @since 2024-11-24
 */
public class AceWebOverrideUrlObject {
    private static final String LOG_TAG = "AceWebOverrideUrlObject";

    private WebResourceRequest request;

    public AceWebOverrideUrlObject(WebResourceRequest request) {
        this.request = request;
    }

    /**
     * Get the request url
     *
     * @return the request url
     */
    public String getRequestUrl() {
        if (this.request.getUrl() != null) {
            return this.request.getUrl().toString();
        }
        return "";
    }
}
