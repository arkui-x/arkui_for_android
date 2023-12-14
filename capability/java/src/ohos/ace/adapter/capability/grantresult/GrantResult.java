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

package ohos.ace.adapter.capability.grantresult;

import android.content.Context;
import ohos.ace.adapter.ALog;

/**
 * GrantResult.
 *
 * @since 1
 */
public class GrantResult {
    private static final String LOG_TAG = "GrantResult";
    private Context mContext;

    /**
     * Permission callbacks
     *
     * @param context the context of application
     */
    public GrantResult(Context context) {
        ALog.i(LOG_TAG, "GrantResult start");
        mContext = context;
    }

    /**
     * Callback for the result from requesting permissions.
     *
     * @param stringArray permissions The requested permissions. Never null.
     * @param intArray grantResults The grant results for the corresponding permissions
     *     which is either {@link android.content.pm.PackageManager#PERMISSION_GRANTED}
     *     or {@link android.content.pm.PackageManager#PERMISSION_DENIED}. Never null.
     */
    public native void onRequestPremissionCallback(String[] stringArray, int[] intArray);
}