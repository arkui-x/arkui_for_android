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
import android.webkit.PermissionRequest;
import ohos.ace.adapter.ALog;

/**
 * AceWebPermissionRequestObject is used to handle permission request from web.
 *
 * @since 2024-05-31
 */
public class AceWebPermissionRequestObject {
    private static final String LOG_TAG = "AceWebPermissionRequestObject";
    private static final int RESOURCESID_ONE = 1;
    private static final int RESOURCESID_TWO = 2;

    private PermissionRequest request;

    public AceWebPermissionRequestObject(PermissionRequest request) {
        this.request = request;
    }

    /**
     * deny the permission request from web.
     */
    public void deny() {
        try {
            this.request.deny();
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call deny method failed");
        }
    }

    /**
     * get the origin of the permission request from web.
     *
     * @return origin of the permission request from web.
     */
    public String getOrigin() {
        Uri uri = this.request.getOrigin();
        if (uri != null) {
            return uri.toString();
        }
        return "";
    }

    /**
     * get the resources of the permission request from web.
     *
     * @return resources of the permission request from web.
     */
    public int getResources() {
        String[] strArray = this.request.getResources();
        int resourcesId = 0;
        for (String str : strArray) {
            if ("android.webkit.resource.VIDEO_CAPTURE".equals(str)) {
                resourcesId |= RESOURCESID_ONE;
            } else if ("android.webkit.resource.AUDIO_CAPTURE".equals(str)) {
                resourcesId |= RESOURCESID_TWO;
            }
        }
        return resourcesId;
    }

    /**
     * grant the permission request from web.
     *
     * @param resourcesId the resourcesId of the permission request from web.
     */
    public void grant(int resourcesId) {
        try {
            this.request.grant(this.request.getResources());
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call grant method failed");
        }
    }
}
