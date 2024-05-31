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

package ohos.ace.adapter.capability.environment;

import android.content.Context;
import android.view.accessibility.AccessibilityManager;

import ohos.ace.adapter.ALog;

/**
 * EnvironmentAosp
 *
 * @since 1
 */
public class EnvironmentAosp extends EnvironmentBase {
    private static final String LOG_TAG = "EnvironmentAosp";
    private Context context;

    /**
     * constructor of EnvironmentPlugin in AOSP platform
     *
     * @param context the context of application
     */
    public EnvironmentAosp(Context context) {
        this.context = context;
        nativeInit();
    }

    @Override
    public String getAccessibilityEnabled() {
        AccessibilityManager manager = (AccessibilityManager) context.getSystemService(Context.ACCESSIBILITY_SERVICE);
        boolean isAccessibilityEnabled = manager.isEnabled() && manager.isTouchExplorationEnabled();
        return isAccessibilityEnabled ? "true" : "false";
    }
}