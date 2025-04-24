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

package ohos.ace.adapter;

import android.content.Context;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import ohos.ace.adapter.capability.editing.TextInputPluginAosp;
import ohos.ace.adapter.capability.surface.AceSurfacePluginAosp;
import ohos.ace.adapter.capability.surface.IAceSurface;
import ohos.ace.adapter.capability.texture.AceTexturePluginAosp;
import ohos.ace.adapter.capability.texture.IAceTexture;

/**
 * AcePlatformPlugin is used to create platform plugin for AceViewAosp.
 *
 * @since 2023-08-06
 */
public class AcePlatformPlugin implements InputConnectionClient {
    private static final String LOG_TAG = "AcePlatformPlugin";

    private AceResourceRegister resRegister;

    private TextInputPluginAosp textInputPlugin;

    /**
     * Constructor of AceViewAosp
     *
     * @param context    Activity context
     * @param instanceId The id of instance
     * @param view       The view which request input
     */
    public AcePlatformPlugin(Context context, int instanceId, View view) {
        ALog.i(LOG_TAG, "AcePlatformPlugin created");

        initResRegister(instanceId);

        textInputPlugin = new TextInputPluginAosp(view, instanceId);
    }

    /**
     * notify keyboard height changed.
     *
     * @param height the height of keyboard
     */
    public void keyboardHeightChanged(int height) {
        if (textInputPlugin != null) {
            textInputPlugin.keyboardHeightChanged(height);
        }
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
     * @param instanceId the instance id
     */
    private void initResRegister(int instanceId) {
        resRegister = new AceResourceRegister();
        long resRegisterPtr = nativeInitResRegister(resRegister, instanceId);
        if (resRegisterPtr == 0L) {
            return;
        }
        resRegister.setRegisterPtr(resRegisterPtr);
    }

    /**
     * Called to release platform plugin.
     */
    public void release() {
        if (resRegister != null) {
            resRegister.release();
        }

        if (textInputPlugin != null) {
            textInputPlugin.release();
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
     * Called to initialize surface plugin.
     *
     * @param context the context
     * @param instanceId the instance id
     */
    public void initSurfacePlugin(Context context, int instanceId) {
        IAceSurface surfaceImpl = new IAceSurface() {
            @Override
            public long attachNaitveSurface(Object surface) {
                ALog.i(LOG_TAG, "AttachNaitveSurface.");
                long nativeSurfacePtr = nativeAttachSurface(surface);
                if (nativeSurfacePtr == 0L) {
                    ALog.e(LOG_TAG, "AttachNaitveSurface failed.");
                }
                return nativeSurfacePtr;
            }
        };
        addResourcePlugin(AceSurfacePluginAosp.createRegister(context, surfaceImpl, instanceId));
    }

    /**
     * notify activity lifecycle changed to plugin.
     *
     * @param isBackground to background state
     */
    public void notifyLifecycleChanged(Boolean isBackground) {
        resRegister.notifyLifecycleChanged(isBackground);
        if (isBackground) {
            textInputPlugin.hideTextInput();
        }
    }

    private native long nativeInitResRegister(AceResourceRegister resRegister, int instanceId);
    private native void nativeRegisterSurface(int instanceId, long textureId, Object surface);
    private native void nativeUnregisterSurface(int instanceId, long textureId);
    private native void nativeRegisterTexture(int instanceId, long textureId, Object surfaceTexture);
    private native void nativeUnregisterTexture(int instanceId, long textureId);
    private native long nativeAttachSurface(Object surface);
}
