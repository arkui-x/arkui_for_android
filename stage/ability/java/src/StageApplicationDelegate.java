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
import android.content.res.AssetManager;

/**
 * This class is the connection between stage application and native layer.
 */
public class StageApplicationDelegate {
    private static final String LOG_TAG = "StageApplicationDelegate";

    public StageApplicationDelegate() {
        Log.i(LOG_TAG, "StageApplicationDelegate()");
    }

    public void init() {
        Log.i(LOG_TAG, "init called");
    }

    public void setNativeAssetManager(AssetManager assetManager) {
        nativeSetAssetManager(assetManager);
    }

    public void setHapPath(String hapPath) {
        nativeSetHapPath(hapPath);
    }

    public void setAssetsFileRelativePath(String path) {
        nativeSetAssetsFileRelativePath(path);
    }

    public void launchApplication() {
        nativeLaunchApplication();
    }

    public void setCacheDir(String cacheDir) {
        nativeSetCacheDir(cacheDir);
    }
    public void setFileDir(String filesDir) {
        nativeSetFileDir(filesDir);
    }

    private native void nativeSetAssetManager(Object assetManager);
    private native void nativeSetHapPath(String hapPath);
    private native void nativeLaunchApplication();
    private native void nativeSetAssetsFileRelativePath(String path);
    private native void nativeSetCacheDir(String cacheDir);
    private native void nativeSetFileDir(String filesDir);
}
