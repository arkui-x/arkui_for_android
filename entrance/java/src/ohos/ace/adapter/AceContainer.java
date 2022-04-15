/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import java.io.FileDescriptor;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Each instance have its own pipeline and thread models, it can contains
 * multiple pages.
 * 
 */
public class AceContainer {
    private static final String LOG_TAG = "AceContainer";

    /**
     * The default value of container type.
     */
    public static final int CONTAINER_TYPE_UNKNOWN = 0;

    /**
     * The type of container, use javascript frontend.
     */
    public static final int CONTAINER_TYPE_JS = 1;

    /**
     * The type of container, use json card frontend.
     */
    public static final int CONTAINER_TYPE_JSON_CARD = 2;

    /**
     * The type of container, use declarative javascript frontend.
     */
    public static final int CONTAINER_TYPE_DECLARATIVE_JS = 3;

    /**
     * Light color mode, keep same with value ColorMode::LIGHT in native.
     */
    public static final int COLOR_MODE_LIGHT = 0;

    /**
     * Dark color mode, keep same with value ColorMode::LIGHT in native.
     */
    public static final int COLOR_MODE_DARK = 1;

    private int instanceId = 0;

    private int type = CONTAINER_TYPE_UNKNOWN;

    private final IAceViewCreator viewCreator;

    private AceEventCallback callback;

    private IAceView view;

    public AceContainer(int instanceId, int type, IAceViewCreator creator, AceEventCallback callback,
        String instanceName) {
        this.instanceId = instanceId;
        this.type = type;
        this.viewCreator = creator;
        this.view = null;
        this.callback = callback;

        nativeCreateContainer(instanceId, type, callback, instanceName);
    }

    /**
     * Destroy this container.
     * 
     */
    public void destroyContainer() {
        nativeDestroyContainer(instanceId);
        if (view != null) {
            view.releaseNativeView();
        }
    }

    /**
     * Get the view which bind to this container
     * 
     * @param density density of defalut display
     * @param widthPixels default display width in pixel.
     * @param heightPixels default display height in pixel.
     * @return the view of container.
     */
    public IAceView getView(float density, int widthPixels, int heightPixels) {
        if (view == null) {
            view = viewCreator.createView(instanceId, density);
            view.initDeviceType();
            nativeSetView(instanceId, view.getNativePtr(), density, widthPixels, heightPixels);
        }
        return view;
    }

    /**
     * Get the view which bind to this container
     * 
     * @return the view of container.
     */
    public IAceView getView() {
        return view;
    }

    /**
     * Start run page with content
     * 
     * @param content content or url for this page.
     * @param params extra params.
     */
    public void loadPageContent(String content, String params) {
        nativeRun(instanceId, content, params);
    }

    /**
     * Start push page with content
     * 
     * @param content content or url for this page.
     * @param params extra params.
     */
    public void pushPageContent(String content, String params) {
        nativePush(instanceId, content, params);
    }

    /**
     * Perform the customized behavior when back pressed.
     * 
     * @return true if the all behavior has been performed, no need super class do anything.
     */
    public boolean onBackPressed() {
        return nativeOnBackPressed(instanceId);
    }

    /**
     * Perform the customized behavior when conatiner on show.
     * 
     */
    public void onShow() {
        nativeOnShow(instanceId);
    }

    /**
     * Perform the customized behavior when conatiner on hide.
     * 
     */
    public void onHide() {
        nativeOnHide(instanceId);
    }

    /**
     * Perform the customized behavior when conatiner on active.
     * 
     */
    public void OnActive() {
        nativeOnActive(instanceId);
    }

    /**
     * Perform the customized behavior when conatiner on inactive.
     * 
     */
    public void OnInactive() {
        nativeOnInactive(instanceId);
    }

    /**
     * Init device info to native.
     * 
     * @param deviceWitdh the witdh of the device
     * @param deviceHeight the height of the device
     * @param orientation the orientation of the device
     * @param density the density of the display
     * @param isRound the screen shape of the display
     * @param mcc the mcc info of the device
     * @param mnc the mnc info of the device
     */
    public void initDeviceInfo(int deviceWitdh, int deviceHeight, int orientation, float density, boolean isRound,
            int mcc, int mnc) {
        nativeInitDeviceInfo(instanceId, deviceWitdh, deviceHeight, orientation, density, isRound, mcc, mnc);
    }

    /**
     * Called when the system has determined to trim the memory.
     * 
     * @param level Indicates the memory trim level, which shows the current memory usage status
     */
    public void onMemoryLevel(int level) {
        nativeOnMemoryLevel(instanceId, level);
    }

    /**
     * Called when the container is alreay running, new start request.
     * 
     * @param dataJson the running data for new request
     */
    public void onNewRequest(String dataJson) {
        if (dataJson == null || dataJson.isEmpty()) {
            return;
        }
        nativeOnNewRequest(instanceId, dataJson);
    }

    /**
     * Add asset path to native.
     * 
     * @param assetManager the AssetManager object
     * @param path the asset path
     */
    public void addAssetPath(Object assetManager, String path) {
        nativeAddAssetPath(instanceId, assetManager, path);
    }

    /**
     * Set font scale to native.
     * 
     * @param fontScale the font scale factor
     */
    public void setFontScale(float fontScale) {
        nativeSetFontScale(instanceId, fontScale);
    }

    /**
     * Update when configuration changed.
     * 
     * @param configData
     */
    public void onConfigurationUpdated(String configData) {
        nativeOnConfigurationUpdated(instanceId, configData);
    }

    /**
     * Set app color mode.
     * 
     * @param colorMode app color mode: light or dark
     */
    public void setColorMode(int colorMode) {
        nativeSetColorMode(instanceId, colorMode);
    }

    /**
     * Set the path for loading app lib
     * 
     * @param libPath app lib path
     */
    public void setLibPath(String libPath) {
        nativeSetLibPath(instanceId, libPath);
    }

    /**
     * Set the host class name.
     * 
     * @param hostClassName
     */
    public void setHostClassName(String hostClassName) {
        nativeSetHostClassName(instanceId, hostClassName);
    }

    /**
     * Init native resouce manager.
     * 
     * @param hapPath path of hap
     * @param themeId theme id of app
     */
    public void initResourceManager(String hapPath, int themeId) {
        nativeInitResourceManager(instanceId, themeId, hapPath);
    }

    /**
     * Get instance id of this container
     * 
     * @return the instance id of this container
     */
    public int getInstanceId() {
        return instanceId;
    }

    private native void nativeCreateContainer(int instanceId, int type, AceEventCallback callback, String instanceName);

    private native void nativeDestroyContainer(int instanceId);

    private native boolean nativeRun(int instanceId, String content, String params);

    private native boolean nativePush(int instanceId, String content, String params);

    private native boolean nativeOnBackPressed(int instanceId);

    private native void nativeOnShow(int instanceId);

    private native void nativeOnHide(int instanceId);

    private native void nativeOnActive(int instanceId);

    private native void nativeOnInactive(int instanceId);

    private native void nativeOnNewRequest(int instanceId, String data);

    private native void nativeOnMemoryLevel(int instanceId, int level);

    private native void nativeAddAssetPath(int instanceId, Object assetManager, String path);

    private native void nativeSetView(int instanceId, long nativeViewPtr, float density, int width, int height);

    private native void nativeInitDeviceInfo(int instanceId, int deviceWitdh, int deviceHeight, int orientation,
        float density, boolean isRound, int mcc, int mnc);

    private native void nativeSetFontScale(int instanceId, float fontSacle);

    private native void nativeSetWindowStyle(int instanceId, int windowModal, int themeType);

    private native void nativeSetSemiModalCustomStyle(int instanceId, int windowHeight, int modalColor);

    private native void nativeSetColorMode(int instanceId, int colorMode);

    private native void nativeSetHostClassName(int instanceId, String hostClassName);

    private native void nativeOnConfigurationUpdated(int instanceId, String configData);

    private native void nativeInitResourceManager(int instanceId, int themeId, String hapPath);

    private native void nativeSetLibPath(int instanceId, String hapPath);
}
