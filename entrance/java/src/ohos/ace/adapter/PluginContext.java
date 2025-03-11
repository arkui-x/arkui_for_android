/**
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

import java.io.File;

import android.content.Context;

import ohos.ace.adapter.capability.bridge.BridgeManager;

/**
 * Plugin Context.
 *
 * provides the resources that the plugin might need.
 *
 * @since 11
 */
public class PluginContext {
    private Context context;
    private BridgeManager bridgeManager;
    private String moduleName;

    /**
     * Constructor of PluginContext.
     *
     * @param context context of the stageActivity.
     * @param bridgeManager bridgeManager of the stageActivity.
     * @param moduleName name of module.
     * @since 11
     */
    public PluginContext(Context context, BridgeManager bridgeManager, String moduleName) {
        this.context = context;
        this.bridgeManager = bridgeManager;
        this.moduleName = moduleName;
    }

    /**
     * get context of the stageActivity.
     *
     * @return context of the stageActivity.
     * @since 11
     */
    public Context getContext() {
        return this.context;
    }

    /**
     * get BridgeManager of the stageActivity.
     *
     * @return BridgeManager of the stageActivity.
     * @since 11
     */
    public BridgeManager getBridgeManager() {
        return this.bridgeManager;
    }

    /**
     * get absolute path of rawfile.
     *
     * @param name name of module.
     * @param filePath relative path of rawfile.
     * @return absolute path of rawfile.
     * @since 11
     */
    public String getRawFilePath(String name, String filePath) {
        String path = this.context.getFilesDir().getAbsolutePath() + "/arkui-x/" + name + "/resources/rawfile/"
            + filePath;
        File file = new File(path);
        if (!file.exists()) {
            return null;
        }
        return path;
    }

    /**
     * get absolute path of rawfile.
     *
     * @param filePath relative path of rawfile.
     * @return absolute path of rawfile.
     * @since 11
     */
    public String getRawFilePath(String filePath) {
        return this.getRawFilePath(this.moduleName, filePath);
    }
}
