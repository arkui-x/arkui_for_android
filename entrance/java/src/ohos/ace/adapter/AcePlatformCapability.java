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

import ohos.ace.adapter.capability.clipboard.ClipboardAosp;
import ohos.ace.adapter.capability.environment.EnvironmentAosp;
import ohos.ace.adapter.capability.font.SystemFontManager;
import ohos.ace.adapter.capability.plugin.PluginManager;
import ohos.ace.adapter.capability.storage.PersistentStorageAosp;
import ohos.ace.adapter.capability.vibrator.VibratorAosp;
import ohos.ace.adapter.DisplayInfo;

/**
 * AceAppPlatformPlugin class is used to provide platform capability
 *
 * @since 2024-06-24
 */
public class AcePlatformCapability {
    private static final String LOG_TAG = "AceAppPlatformPlugin";

    private ClipboardAosp clipboardAosp;

    private EnvironmentAosp environmentAosp;

    private SystemFontManager systemFontManager;

    private PersistentStorageAosp persistentStorage;

    private VibratorAosp vibratorAosp;

    private PluginManager pluginManager;

    /**
     * Constructor of AceAppPlatformPlugin
     *
     * @param context Application context
     */
    public AcePlatformCapability(Context context) {
        ALog.i(LOG_TAG, "AceAppPlatformPlugin created");

        clipboardAosp = new ClipboardAosp(context);

        environmentAosp = new EnvironmentAosp(context);

        systemFontManager = new SystemFontManager();

        persistentStorage = new PersistentStorageAosp(context);

        vibratorAosp = new VibratorAosp(context);

        pluginManager = new PluginManager(context);

        DisplayInfo.getInstance().setContext(context);
    }
}
