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

import android.content.res.AssetManager;
import android.util.Log;

/**
 * This class is responsible for communicating with the stage application delegate jni.
 */
public class StageApplicationDelegate {
    private static final String LOG_TAG = "StageApplicationDelegate";

    /**
     * Constructor.
     */
    public StageApplicationDelegate() {
        Log.i(LOG_TAG, "Constructor called.");
    }

    /**
     * Set asset manager object to native.
     *
     * @param assetManager the asset manager.
     */
    public void setNativeAssetManager(AssetManager assetManager) {
        nativeSetAssetManager(assetManager);
    }

    /**
     * Set hap path to native.
     *
     * @param hapPath the hap path.
     */
    public void setHapPath(String hapPath) {
        nativeSetHapPath(hapPath);
    }

    /**
     * Set assets file relative path to native.
     *
     * @param path the assets file relative path.
     */
    public void setAssetsFileRelativePath(String path) {
        nativeSetAssetsFileRelativePath(path);
    }

    /**
     * Launch application.
     */
    public void launchApplication() {
        nativeLaunchApplication();
    }

    /**
     * Set cache dir to native.
     *
     * @param cacheDir the cache dir.
     */
    public void setCacheDir(String cacheDir) {
        nativeSetCacheDir(cacheDir);
    }

    /**
     * Set file dir to native.
     *
     * @param filesDir the file dir.
     */
    public void setFileDir(String filesDir) {
        nativeSetFileDir(filesDir);
    }

    /**
     * Set resources file prefix path to native.
     *
     * @param path the resources file prefix path.
     */
    public void setResourcesFilePrefixPath(String path) {
        nativeSetResourcesFilePrefixPath(path);
    }

    /**
     * Set pid and uid to native.
     *
     * @param pid the process id.
     * @param uid the uid.
     */
    public void setPidAndUid(int pid, int uid) {
        nativeSetPidAndUid(pid, uid);
    }

    /**
     * Init configuration.
     *
     * @param data the configuration data.
     */
    public void initConfiguration(String data) {
        nativeInitConfiguration(data);
    }

    /**
     * Called by the system when the device configuration changes while your component is running.
     *
     * @param data the configuration data.
     */
    public void onConfigurationChanged(String data) {
        nativeOnConfigurationChanged(data);
    }

    /**
     * Set locale to native.
     *
     * @param language the language.
     * @param country the country.
     * @param script the script.
     */
    public void setLocale(String language, String country, String script) {
        nativeSetLocale(language, country, script);
    }

    private native void nativeSetAssetManager(Object assetManager);
    private native void nativeSetHapPath(String hapPath);
    private native void nativeLaunchApplication();
    private native void nativeSetAssetsFileRelativePath(String path);
    private native void nativeSetCacheDir(String cacheDir);
    private native void nativeSetFileDir(String filesDir);
    private native void nativeSetResourcesFilePrefixPath(String path);
    private native void nativeSetPidAndUid(int pid, int uid);
    private native void nativeInitConfiguration(String data);
    private native void nativeOnConfigurationChanged(String data);
    private native void nativeSetLocale(String language, String country, String script);
}
