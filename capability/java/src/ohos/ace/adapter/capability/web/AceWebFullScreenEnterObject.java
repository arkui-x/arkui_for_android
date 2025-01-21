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

import ohos.ace.adapter.ALog;
import android.webkit.WebChromeClient;
/**
 * Javadoc annotation for the AceWebFullScreenEnterObject class.
 *
 * @since 2024-12-30
 */
public class AceWebFullScreenEnterObject {
    private static final String LOG_TAG = "AceWebFullScreenEnterObject";
    private WebChromeClient.CustomViewCallback mCallBack;
    private int videoWidth;
    private int videoHeight;

    public AceWebFullScreenEnterObject(int videoWidth, int videoHeight) {
        this.videoWidth = videoWidth;
        this.videoHeight = videoHeight;
    }

    public int widths() {
        return this.videoWidth;
    }

    public int heights() {
        return this.videoHeight;
    }

    public void setFullEnterRequestExitCallback(WebChromeClient.CustomViewCallback callback) {
        this.mCallBack = callback;
    }

    /**
     * Set full screen access request exit callback.
     *
     */
    public void exitFullScreen() {
        try {
            if (this.mCallBack != null) {
                this.mCallBack.onCustomViewHidden();
                this.mCallBack = null;
            }
        } catch (Exception e) {
            ALog.w(LOG_TAG, "exitFullScreen Exception error");
        }
    }
}