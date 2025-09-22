/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

import ohos.ace.adapter.DisplayManagerAgent;
import android.content.Intent;
import ohos.ace.adapter.WindowViewInterface;
import java.util.List;
import java.util.ArrayList;

/**
 * This class is responsible for communicating with the stage activity delegate jni.
 *
 * @since 1
 */
public class StageActivityDelegate {
    private static final String LOG_TAG = "StageActivityDelegate";
    private static final List<INTENTCALLBACK> intentCallbackList = new ArrayList<>();

    /**
     * The interface of intent callback.
     */
    public interface INTENTCALLBACK {
        /**
         * Callback method when the intent is triggered.
         *
         * @param requestCode the request code.
         * @param resultCode  the result code.
         * @param intent      the intent.
         */
        void onResult(int requestCode, int resultCode, Intent intent);
    }

    /**
     * Add intent callback.
     *
     * @param callback the intent callback.
     */
    public static void addIntentCallback(INTENTCALLBACK callback) {
        if (callback != null) {
            intentCallbackList.add(callback);
        }
    }

    /**
     * Trigger the intent callback.
     *
     * @param requestCode the request code.
     * @param resultCode  the result code.
     * @param intent      the intent.
     */
    public static void triggerIntentCallback(int requestCode, int resultCode, Intent intent) {
        if (intentCallbackList != null) {
            for (INTENTCALLBACK callback : intentCallbackList) {
                callback.onResult(requestCode, resultCode, intent);
            }
        }
    }

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
        SubWindowManager.getInstance().setActivity(object, instanceName);
        DisplayManagerAgent.getInstance().setActivity(object);
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
        SubWindowManager.getInstance().releaseActivity(instanceName);
    }

    /**
     * Dispatch the onnewwant lifecycle to native.
     *
     * @param instanceName the activity instance name.
     * @param params       the want params.
     */
    public void dispatchOnNewWant(String instanceName, String params) {
        Log.i(LOG_TAG, "dispatchOnNewWant called");
        nativeDispatchOnNewWant(instanceName, params);
    }

    /**
     * Dispatch the onforeground lifecycle to native.
     *
     * @param instanceName the activity instance name.
     * @param object       the stage activity.
     */
    public void dispatchOnForeground(String instanceName, StageActivity object) {
        Log.i(LOG_TAG, "DispatchOnForeground called");
        nativeDispatchOnForeground(instanceName);
        SubWindowManager.getInstance().setActivity(object, instanceName);
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
    public void setWindowView(String instanceName, WindowViewInterface windowView) {
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
     * @param intent the intent returned after the ability is destroyed.
     */
    public void dispatchOnActivityResult(
        String instanceName, int requestCode, int resultCode, String resultWantParams, Intent intent) {
        Log.i(LOG_TAG, "dispatchOnActivityResult called");
        triggerIntentCallback(requestCode, resultCode, intent);
        nativeDispatchOnAbilityResult(instanceName, requestCode, resultCode, resultWantParams);
    }

    /**
     * dispatch fold status change event
     *
     * @param instanceName the activity instance name.
     * @param foldStatus the fold status.
     */
    public void dispatchFoldStatusChange(String instanceName, int foldStatus) {
        nativeFoldStatusChangeCallback(instanceName, foldStatus);
    }

    private native void nativeAttachStageActivity(String instanceName, StageActivity object);

    private native void nativeDispatchOnCreate(String instanceName, String params);

    private native void nativeDispatchOnDestroy(String instanceName);

    private native void nativeDispatchOnForeground(String instanceName);

    private native void nativeDispatchOnBackground(String instanceName);

    private native void nativeDispatchOnNewWant(String instanceName, String params);

    private native void nativeSetWindowView(String instanceName, WindowViewInterface windowView);

    private native void nativeCreateAbilityDelegator(String bundleName, String moduleName,
                                                        String testRunerName, String timeout);

    private native void nativeDispatchOnAbilityResult(
        String instanceName, int requestCode, int resultCode, String resultWantParams);

    private native void nativeFoldStatusChangeCallback(String instanceName, int foldStatus);
}
