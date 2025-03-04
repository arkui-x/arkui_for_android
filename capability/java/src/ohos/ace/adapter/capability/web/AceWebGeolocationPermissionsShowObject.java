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
import ohos.ace.adapter.ALog;

import android.webkit.GeolocationPermissions;

/**
 * AceGeolocationPermissionsShowObject class is used to handle Web page geolocation permission display objects.
 *
 * @since 1
 */
public class AceWebGeolocationPermissionsShowObject {
    private static final String LOG_TAG = "AceWebGeolocationPermissionsShowObject";

    private String origin;
    private boolean isIncognitoMode = false;
    private GeolocationPermissions.Callback callback;

    public AceWebGeolocationPermissionsShowObject(String origin, GeolocationPermissions.Callback callback,
            boolean isIncognitoMode) {
        this.origin = origin;
        this.callback = callback;
        this.isIncognitoMode = isIncognitoMode;
    }

    public String getOrigin() {
        return this.origin;
    }

    /**
     * invoke method is used to call the callback method.
     *
     * @param origin The origin of the web page.
     * @param allow Whether to allow geolocation permission.
     * @param retain Whether to retain the geolocation permission.
     */
    public void invoke(String origin, boolean allow, boolean retain) {
        try {
            if (isIncognitoMode) {
                this.callback.invoke(origin, allow, false);
            } else {
                this.callback.invoke(origin, allow, retain);
            }
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call invoke method failed");
        }
    }
}
