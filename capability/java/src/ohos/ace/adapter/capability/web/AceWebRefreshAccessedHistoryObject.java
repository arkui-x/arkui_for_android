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
/**
 * Javadoc annotation for the AceWebRefreshAccessedHistoryObject class.
 *
 * @since 2024-12-30
 */
public class AceWebRefreshAccessedHistoryObject {
    private static final String LOG_TAG = "AceWebRefreshAccessedHistoryObject";

    private String url;
    private boolean isRefreshed;

    public AceWebRefreshAccessedHistoryObject(String url, boolean isRefreshed) {
        this.url = url;
        this.isRefreshed = isRefreshed;
    }

    public String getUrl() {
        return this.url;
    }

    public boolean getIsRefreshed() {
        return this.isRefreshed;
    }
}