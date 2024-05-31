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

import ohos.ace.adapter.capability.clipboard.ClipboardPluginAosp;
import ohos.ace.adapter.capability.editing.TextInputPluginAosp;
import ohos.ace.adapter.capability.environment.EnvironmentAosp;
import ohos.ace.adapter.capability.plugin.PluginManager;
import ohos.ace.adapter.capability.storage.PersistentStorageAosp;
import ohos.ace.adapter.capability.texture.AceTexturePluginAosp;
import ohos.ace.adapter.capability.texture.IAceTexture;
import ohos.ace.adapter.capability.vibrator.VibratorPluginAosp;

public class AcePlatformPlugin implements InputConnectionClient {
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
    public AcePlatformPlugin(Context context, int instanceId, View view) {
        ALog.i(LOG_TAG, "AcePlatformPlugin created");

        initResRegister(instanceId);

        clipboardPlugin = new ClipboardPluginAosp(context);
        textInputPlugin = new TextInputPluginAosp(view, instanceId);
        environmentPlugin = new EnvironmentAosp(context);
        persistentStoragePlugin = new PersistentStorageAosp(context);
        vibratorPlugin = new VibratorPluginAosp(context);
        pluginManager = new PluginManager(context);
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
    private void initResRegister(int instanceId) {
        resRegister = new AceResourceRegister();
        long resRegisterPtr = nativeInitResRegister(resRegister, instanceId);
        if (resRegisterPtr == 0L) {
            return;
        }
        resRegister.setRegisterPtr(resRegisterPtr);
    }

    public void releseResRegister(int instanceId) {
        if (resRegister != null) {
            resRegister.release();
        }
    }

    @Override
    public InputConnection onCreateInputConnection(View view, EditorInfo outAttrs) {
        if (textInputPlugin != null) {
            return textInputPlugin.createInputConnection(view, outAttrs);
        }
        return null;
    }

    /**
     * Called to initialize texture plugin.
     *
     * @param instanceId the instance id
     */
    public void initTexturePlugin(int instanceId) {
        IAceTexture textureImpl = new IAceTexture() {
            @Override
            public void registerSurface(long textureId, Object surface) {
                ALog.i(LOG_TAG, "registerSurface.");
                nativeRegisterSurface(instanceId, textureId, surface);
            }

            @Override
            public void registerTexture(long textureId, Object surfaceTexture) {
                ALog.i(LOG_TAG, "registerTexture.");
                nativeRegisterTexture(instanceId, textureId, surfaceTexture);
            }

            @Override
            public void markTextureFrameAvailable(long textureId) {
            }

            @Override
            public void unregisterTexture(long textureId) {
                ALog.i(LOG_TAG, "unregisterTexture.");
                nativeUnregisterTexture(instanceId, textureId);
            }

            @Override
            public void unregisterSurface(long textureId) {
                ALog.i(LOG_TAG, "unregisterSurface.");
                nativeUnregisterSurface(instanceId, textureId);
            }
        };
        addResourcePlugin(AceTexturePluginAosp.createRegister(instanceId, textureImpl));
    }

    /**
     * notify activity lifecycle changed to plugin.
     *
     * @param isBackground to background state
     */
    public void notifyLifecycleChanged(Boolean isBackground) {
        resRegister.notifyLifecycleChanged(isBackground);
    }

    private native long nativeInitResRegister(AceResourceRegister resRegister, int instanceId);
    private native void nativeRegisterSurface(int instanceId, long textureId, Object surface);
    private native void nativeUnregisterSurface(int instanceId, long textureId);
    private native void nativeRegisterTexture(int instanceId, long textureId, Object surfaceTexture);
    private native void nativeUnregisterTexture(int instanceId, long textureId);
}
