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

package ohos.ace.adapter;

import android.content.Context;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import ohos.ace.adapter.capability.bridge.BridgeManager;
import ohos.ace.adapter.capability.clipboard.ClipboardPluginAosp;
import ohos.ace.adapter.capability.editing.TextInputPluginAosp;
import ohos.ace.adapter.capability.environment.EnvironmentAosp;
import ohos.ace.adapter.capability.storage.PersistentStorageAosp;
import ohos.ace.adapter.capability.vibrator.VibratorPluginAosp;
import ohos.ace.adapter.capability.plugin.PluginManager;

public class AcePlatformPlugin {
    private static final String LOG_TAG = "AcePlatformPlugin";

    private AceResourceRegister resRegister;

    private ClipboardPluginAosp clipboardPlugin;

    private TextInputPluginAosp textInputPlugin;

    private EnvironmentAosp environmentPlugin;

    private PersistentStorageAosp persistentStoragePlugin;

    private VibratorPluginAosp vibratorPlugin;

    private PluginManager pluginManager;

    /**
     * Constructor of AceViewAosp
     *
     * @param context    Application context
     * @param instanceId The id of instance
     * @param view       The view which request input
     */
    public AcePlatformPlugin(Context context, int instanceId, View view, long nativeViewPtr) {
        ALog.i(LOG_TAG, "AcePlatformPlugin created");

        initResRegister(nativeViewPtr, instanceId);

        clipboardPlugin = new ClipboardPluginAosp(context);
        textInputPlugin = new TextInputPluginAosp(view, instanceId);
        environmentPlugin = new EnvironmentAosp(context);
        persistentStoragePlugin = new PersistentStorageAosp(context);
        vibratorPlugin = new VibratorPluginAosp(context);
        pluginManager = new PluginManager(context);
        BridgeManager bridgeManager = BridgeManager.getInstance();
        bridgeManager.nativeInit();
    }

    /**
     * Called to add resource plugin.
     *
     * @param plugin the plugin
     */
    public void addResourcePlugin(AceResourcePlugin plugin) {
        if (resRegister != null) {
            resRegister.registerPlugin(plugin);
        }
    }

    /**
     * Initialize resource register
     *
     */
    private void initResRegister(long nativeViewPtr, int instanceId) {
        resRegister = new AceResourceRegister();
        if (nativeViewPtr == 0L) {
            ALog.e(LOG_TAG, "initResRegister nativeViewPtr is null");
            return;
        }
        long resRegisterPtr = nativeInitResRegister(nativeViewPtr, resRegister, instanceId);
        if (resRegisterPtr == 0L) {
            return;
        }
        resRegister.setRegisterPtr(resRegisterPtr);
    }

    public InputConnection onCreateInputConnection(View view, EditorInfo outAttrs) {
        if (textInputPlugin != null) {
            return textInputPlugin.createInputConnection(view, outAttrs);
        }
        return null;
    }

    private native long nativeInitResRegister(long viewPtr, AceResourceRegister resRegister, int instanceId);
}
