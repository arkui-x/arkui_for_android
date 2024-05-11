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

package ohos.ace.adapter;

import android.content.Context;

import ohos.ace.adapter.capability.clipboard.ClipboardPluginAosp;
import ohos.ace.adapter.capability.environment.EnvironmentAosp;
import ohos.ace.adapter.capability.font.SystemFontManager;
import ohos.ace.adapter.capability.plugin.PluginManager;
import ohos.ace.adapter.capability.storage.PersistentStorageAosp;
import ohos.ace.adapter.capability.vibrator.VibratorPluginAosp;
import ohos.ace.adapter.DisplayInfo;

public class AceAppPlatformPlugin {
    private static final String LOG_TAG = "AceAppPlatformPlugin";

    private ClipboardPluginAosp clipboardPlugin;

    private EnvironmentAosp environmentPlugin;

    private SystemFontManager systemFontManager;

    private PersistentStorageAosp persistentStoragePlugin;

    private VibratorPluginAosp vibratorPlugin;

    private PluginManager pluginManager;

    /**
     * Constructor of AceAppPlatformPlugin
     *
     * @param context Application context
     */
    public AceAppPlatformPlugin(Context context) {
        ALog.i(LOG_TAG, "AceAppPlatformPlugin created");

        clipboardPlugin = new ClipboardPluginAosp(context);

        environmentPlugin = new EnvironmentAosp(context);

        systemFontManager = new SystemFontManager();

        persistentStoragePlugin = new PersistentStorageAosp(context);

        vibratorPlugin = new VibratorPluginAosp(context);

        pluginManager = new PluginManager(context);

        DisplayInfo.getInstance().setContext(context);
    }
}
