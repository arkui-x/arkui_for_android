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

import android.os.Trace;
import android.util.Log;

import ohos.ace.adapter.WindowView;
import ohos.ace.adapter.DisplayInfo;

/**
 * This class is responsible for communicating with the stage activity delegate jni.
 *
 * @since 1
 */
public class StageActivityDelegate {
    private static final String LOG_TAG = "StageActivityDelegate";

    /**
     * Constructor.
     */
    public StageActivityDelegate() {
        Log.i(LOG_TAG, "Constructor called.");
    }

    /**
     * Attach stage activity to native.
     *
     * @param instanceName the activity instance name.
     * @param object       the stage activity.
     */
    public void attachStageActivity(String instanceName, StageActivity object) {
        Trace.beginSection("attachStageActivity");
        nativeAttachStageActivity(instanceName, object);
        SubWindowManager.getInstance().setActivity(object);
        DisplayInfo.getInstance().setContext(object);
        Trace.endSection();
    }

    /**
     * Dispatch the oncreate lifecycle to native.
     *
     * @param instanceName the activity instance name.
     * @param params       the want params
     */
    public void dispatchOnCreate(String instanceName, String params) {
        Trace.beginSection("attachStageActivity");
        nativeDispatchOnCreate(instanceName, params);
        Trace.endSection();
    }

    /**
     * Dispatch the ondestroy lifecycle to native.
     *
     * @param instanceName the activity instance name.
     */
    public void dispatchOnDestroy(String instanceName) {
        Log.i(LOG_TAG, "dispatchOnDestroy called");
        nativeDispatchOnDestroy(instanceName);
    }

    /**
     * Dispatch the onnewwant lifecycle to native.
     *
     * @param instanceName the activity instance name.
     */
    public void dispatchOnNewWant(String instanceName) {
        Log.i(LOG_TAG, "dispatchOnNewWant called");
        nativeDispatchOnNewWant(instanceName);
    }

    /**
     * Dispatch the onforeground lifecycle to native.
     *
     * @param instanceName the activity instance name.
     */
    public void dispatchOnForeground(String instanceName) {
        Log.i(LOG_TAG, "DispatchOnForeground called");
        nativeDispatchOnForeground(instanceName);
    }

    /**
     * Dispatch the onbackground lifecycle to native.
     *
     * @param instanceName the activity instance name.
     */
    public void dispatchOnBackground(String instanceName) {
        Log.i(LOG_TAG, "DispatchOnBackground called");
        nativeDispatchOnBackground(instanceName);
    }

    /**
     * Set window view to native.
     *
     * @param instanceName the activity instance name.
     * @param windowView   the window view.
     */
    public void setWindowView(String instanceName, WindowView windowView) {
        Trace.beginSection("setWindowView");
        nativeSetWindowView(instanceName, windowView);
        Trace.endSection();
    }

    /**
     * Create ability delegator.
     *
     * @param bundleName the bundle name.
     * @param moduleName the module name.
     * @param testRunerName the testRuner name.
     * @param timeout timeout.
     */
    public void createAbilityDelegator(String bundleName, String moduleName, String testRunerName, String timeout) {
        nativeCreateAbilityDelegator(bundleName, moduleName, testRunerName, timeout);
    }

    /**
     * Dispatch the onAbilityResult to native.
     *
     * @param instanceName the activity instance name.
     * @param requestCode the request code returned after the ability is started.
     * @param resultCode the result code returned after the ability is destroyed.
     * @param resultWantParams the data returned after the ability is destroyed.
     */
    public void dispatchOnActivityResult(
        String instanceName, int requestCode, int resultCode, String resultWantParams) {
        Log.i(LOG_TAG, "dispatchOnActivityResult called");
        nativeDispatchOnAbilityResult(instanceName, requestCode, resultCode, resultWantParams);
    }

    private native void nativeAttachStageActivity(String instanceName, StageActivity object);

    private native void nativeDispatchOnCreate(String instanceName, String params);

    private native void nativeDispatchOnDestroy(String instanceName);

    private native void nativeDispatchOnForeground(String instanceName);

    private native void nativeDispatchOnBackground(String instanceName);

    private native void nativeDispatchOnNewWant(String instanceName);

    private native void nativeSetWindowView(String instanceName, WindowView windowView);

    private native void nativeCreateAbilityDelegator(String bundleName, String moduleName,
                                                        String testRunerName, String timeout);

    private native void nativeDispatchOnAbilityResult(
        String instanceName, int requestCode, int resultCode, String resultWantParams);
}
