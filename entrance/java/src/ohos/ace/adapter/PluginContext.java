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

    /**
     * Constructor of PluginContext.
     * 
     * @param context context of the stageActivity.
     * @param bridgeManager bridgeManager of the stageActivity.
     * @since 11
     */
    public PluginContext(Context context, BridgeManager bridgeManager) {
        this.context = context;
        this.bridgeManager = bridgeManager;
    }

    /**
     * get context of the stageActivity.
     * 
     * @since 11
     */
    public Context getContext() {
        return this.context;
    }

    /**
     * get BridgeManager of the stageActivity.
     * 
     * @since 11
     */
    public BridgeManager getBridgeManager() {
        return this.bridgeManager;
    }
}
