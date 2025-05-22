/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import ohos.ace.adapter.WindowViewInterface;
import ohos.ace.adapter.DisplayInfo;
import ohos.ace.adapter.DisplayManagerAgent;
import ohos.stage.ability.adapter.StageFragment;
import ohos.stage.ability.adapter.SubWindowManager;

/**
 * This class is responsible for communicating with the stage fragment delegate jni.
 *
 * @since 1
 */
public class StageFragmentDelegate {
    private static final String LOG_TAG = "StageFragmentDelegate";

    /**
     * Constructor.
     */
    public StageFragmentDelegate() {
        Log.i(LOG_TAG, "Constructor called.");
    }

    /**
     * Attach stage fragment to native.
     *
     * @param instanceName      the fragment instance name.
     * @param fragment          currently fragment
     */
    public void attachStageFragment(String instanceName, StageFragment fragment) {
        Trace.beginSection("attachStageFragment");
        SubWindowManager.getInstance().setActivity(fragment.getActivity(), instanceName);
        DisplayManagerAgent.getInstance().setActivity(fragment.getActivity());
        DisplayInfo.getInstance().setContext(fragment.getActivity());
        nativeAttachFragment(instanceName, fragment);
        Trace.endSection();
    }

    /**
     * Set window view to native.
     *
     * @param instanceName the fragment instance name.
     * @param windowView   the window view.
     */
    public void setWindowView(String instanceName, WindowViewInterface windowView) {
        Trace.beginSection("setWindowView");
        nativeSetWindowView(instanceName, windowView);
        Trace.endSection();
    }

    /**
     * Dispatch the onCreate lifecycle to native.
     *
     * @param instanceName the fragment instance name.
     * @param params       the want params
     */
    public void dispatchOnCreate(String instanceName, String params) {
        Trace.beginSection("dispatchOnCreate");
        nativeDispatchOnCreate(instanceName, params);
        Trace.endSection();
    }

    /**
     * Dispatch the onForeground lifecycle to native.
     *
     * @param instanceName the fragment instance name.
     * @param fragment     currently fragment
     */
    public void dispatchOnForeground(String instanceName, StageFragment fragment) {
        SubWindowManager.getInstance().setActivity(fragment.getActivity(), instanceName);
        nativeDispatchOnForeground(instanceName);
    }

    /**
     * Dispatch the onBackground lifecycle to native.
     *
     * @param instanceName the fragment instance name.
     */
    public void dispatchOnBackground(String instanceName) {
        nativeDispatchOnBackground(instanceName);
    }

    /**
     * Dispatch the onDestroy lifecycle to native.
     *
     * @param instanceName the fragment instance name.
     */
    public void dispatchOnDestroy(String instanceName) {
        nativeDispatchOnDestroy(instanceName);
        SubWindowManager.getInstance().releaseActivity(instanceName);
    }

    private native void nativeSetWindowView(String instanceName, WindowViewInterface windowView);

    private native void nativeDispatchOnCreate(String instanceName, String params);

    private native void nativeDispatchOnForeground(String instanceName);

    private native void nativeDispatchOnBackground(String instanceName);

    private native void nativeDispatchOnDestroy(String instanceName);

    private native void nativeAttachFragment(String instanceName, StageFragment fragemt);
}