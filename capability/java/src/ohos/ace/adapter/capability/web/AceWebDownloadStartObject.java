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


public class AceWebDownloadStartObject {
    private static final String LOG_TAG = "AceWebDownloadStartObject";

    private String url;
    private String userAgent;
    private String contentDisposition;
    private String mimetype;
    private long contentLength;

    public AceWebDownloadStartObject(String url, String userAgent, String contentDisposition, String mimetype, long contentLength) {
        this.url = url;
        this.userAgent = userAgent;
        this.contentDisposition = contentDisposition;
        this.mimetype = mimetype;
        this.contentLength = contentLength;
    }

    public String getUrl() {
        return this.url;
    }

    public String getUserAgent() {
        return this.userAgent;
    }

    public String getContentDisposition() {
        return this.contentDisposition;
    }

    public String getMimetype() {
        return this.mimetype;
    }

    public long getContentLength() {
        return this.contentLength;
    }
}
