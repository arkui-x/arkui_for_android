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

import android.net.Uri;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import ohos.ace.adapter.ALog;

public class AceWebFileChooserObject {
    private static final String LOG_TAG = "AceWebFileChooserObject";

    private ValueCallback<Uri[]> filePathCallback;
    private WebChromeClient.FileChooserParams fileChooserParams;

    public AceWebFileChooserObject(ValueCallback<Uri[]> filePathCallback,
            WebChromeClient.FileChooserParams fileChooserParams) {
        this.filePathCallback = filePathCallback;
        this.fileChooserParams = fileChooserParams;
    }

    public String getTitle() {
        if (this.fileChooserParams.getTitle() != null) {
            return this.fileChooserParams.getTitle().toString();
        }
        return "";
    }

    public int getMode() {
        return this.fileChooserParams.getMode();
    }

    public String[] getAcceptType() {
        return this.fileChooserParams.getAcceptTypes();
    }

    public boolean isCapture() {
        return this.fileChooserParams.isCaptureEnabled();
    }

    public void handleFileList(String[] fileList) {
        Uri[] uri = new Uri[fileList.length];
        for (int index = 0; index < fileList.length; ++index) {
            uri[index] = Uri.parse(fileList[index]);
        }
        try {
            this.filePathCallback.onReceiveValue(uri);
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call onReceiveValue method failed");
        }
    }
}
