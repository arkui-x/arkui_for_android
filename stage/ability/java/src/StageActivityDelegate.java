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

package ohos.stage.ability.adapter;

import android.util.Log;
import ohos.ace.adapter.WindowView;

public class StageActivityDelegate {
    private static final String LOG_TAG = "StageActivityDelegate";
    public StageActivityDelegate() {
        Log.i(LOG_TAG, "StageActivityDelegate()");
    }

    public void attachStageActivity(StageActivity object) {
        nativeAttachStageActivity(object);
    }

    public void dispatchOnCreate(String instanceName) {
        Log.i(LOG_TAG, "dispatchOnCreate called");
        nativeDispatchOnCreate(instanceName);
    }

    public void dispatchOnDestroy(String instanceName) {
        Log.i(LOG_TAG, "dispatchOnDestroy called");
        nativeDispatchOnDestroy(instanceName);
    }

    public void dispatchOnNewWant(String instanceName) {
        Log.i(LOG_TAG, "dispatchOnNewWant called");
        nativeDispatchOnNewWant(instanceName);
    }

    public void dispatchOnForeground(String instanceName) {
        Log.i(LOG_TAG, "DispatchOnForeground called");
        nativeDispatchOnForeground(instanceName);
    }

    public void dispatchOnBackground(String instanceName) {
        Log.i(LOG_TAG, "DispatchOnBackground called");
        nativeDispatchOnBackground(instanceName);
    }

    public void SetWindowView(String instanceName, WindowView windowView) {
        Log.i(LOG_TAG, "SetWindowView called");
        nativeSetWindowView(instanceName, windowView);
    }

    private native void nativeAttachStageActivity(StageActivity object);
    private native void nativeDispatchOnCreate(String instanceName);
    private native void nativeDispatchOnDestroy(String instanceName);
    private native void nativeDispatchOnForeground(String instanceName);
    private native void nativeDispatchOnBackground(String instanceName);
    private native void nativeDispatchOnNewWant(String instanceName);
    private native void nativeSetWindowView(String instanceName, WindowView windowView);
}
