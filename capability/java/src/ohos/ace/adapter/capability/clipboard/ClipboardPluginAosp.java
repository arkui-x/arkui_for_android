/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.clipboard;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;

import ohos.ace.adapter.ALog;

/**
 * ClipboardPluginAosp
 *
 * @since 1
 */
public class ClipboardPluginAosp extends ClipboardPluginBase {
    private static final String LOG_TAG = "ClipboardPlugin";

    private final ClipboardManager clipManager;

    /**
     * ClipboardPlugin on AOSP platform
     *
     * @param context context of the application
     */
    public ClipboardPluginAosp(Context context) {
        if (context != null) {
            Object service = context.getSystemService(Context.CLIPBOARD_SERVICE);
            if (service instanceof ClipboardManager) {
                this.clipManager = (ClipboardManager) service;
            } else {
                ALog.e(LOG_TAG, "unable to get clipboard service");
                this.clipManager = null;
            }
        } else {
            ALog.e(LOG_TAG, "context is null");
            this.clipManager = null;
        }
        nativeInit();
    }

    @Override
    public String getData() {
        if (clipManager != null) {
            ClipData clipData = clipManager.getPrimaryClip();
            CharSequence charSequence = null;
            if (clipData != null && clipData.getItemCount() > 0) {
                charSequence = clipData.getItemAt(0).getText();
            }
            if (charSequence != null) {
                return charSequence.toString();
            }
        }
        return "";
    }

    @Override
    public void setData(String data) {
        if (clipManager != null) {
            ClipData clipData = ClipData.newPlainText(null, data);
            clipManager.setPrimaryClip(clipData);
        }
    }

    @Override
    public boolean hasData() {
        if (clipManager != null) {
            return clipManager.hasPrimaryClip();
        }
        return false;
    }

    @Override
    public void clear() {
        if (clipManager != null) {
            clipManager.setPrimaryClip(ClipData.newPlainText(null, null));
        }
    }
}
