/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.plugin;

import ohos.ace.adapter.ALog;

import android.content.Context;
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;

/**
 * PluginManager is an add-on for handling register external plugins.
 *
 * @since 1
 */
public class PluginManager {
    private static final String LOG_TAG = "Ace_PluginManager";

    private final Context context;

    /**
     * constructor of PluginManager in AOSP platform
     */
    public PluginManager(Context context) {
        this.context = context;
        nativeInit();
    }

    public void registerPlugin(long pluginRegisterFunc, String pluginPackageName) {
        nativeRegister(pluginRegisterFunc, pluginPackageName);
    }

    public void initPlugin(String packageName) {
        try {
            ALog.d(LOG_TAG, "init plugin: " + packageName);
            Class pluginClass = Class.forName(packageName);
            Constructor constructorMethod = pluginClass.getConstructor(Context.class);

            // take the object reference in Native side
            Object object = constructorMethod.newInstance(context);
        } catch (Exception e) {
            ALog.e(LOG_TAG, "init plugin failed: " + e);
        }
    }

    /**
     * native func for Init.
     */
    private native void nativeInit();
    private native void nativeRegister(long pluginRegisterFunc, String pluginPackageName);
}
