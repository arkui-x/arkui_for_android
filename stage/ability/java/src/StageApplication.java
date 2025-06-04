/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

import android.app.Application;
import android.content.Context;
import android.content.res.Configuration;
import android.database.ContentObserver;
import android.os.Handler;
import android.provider.Settings;
import android.util.Log;

/**
 * This class extends from Android Application, the entry of app
 *
 * @since 1
 */
public class StageApplication extends Application {
    private static final String LOG_TAG = "StageApplication";
    private StageApplicationDelegate appDelegate = null;

    private ContentObserver fontScaleObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            float fontScale = Settings.System.getFloat(getContentResolver(), Settings.System.FONT_SCALE, 1.0f);
            Configuration config = getResources().getConfiguration();
            config.fontScale = fontScale;
            appDelegate.onConfigurationChanged(config);
        }
    };

    /**
     * Call when Application is created.
     */
    @Override
    public void onCreate() {
        Log.i(LOG_TAG, "StageApplication onCreate called");
        super.onCreate();

        appDelegate = new StageApplicationDelegate();
        appDelegate.initApplication(this);
        getContentResolver().registerContentObserver(Settings.System.getUriFor(Settings.System.FONT_SCALE), true,
            fontScaleObserver);
    }

    /**
     * Call when Application is on configuration changed.
     *
     * @param newConfig the configuration.
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        Log.i(LOG_TAG, "StageApplication onConfigurationChanged called");
        super.onConfigurationChanged(newConfig);
        if (appDelegate == null) {
            Log.e(LOG_TAG, "appDelegate is null");
            return;
        }
        newConfig.fontScale = Settings.System.getFloat(getContentResolver(), Settings.System.FONT_SCALE, 1.0f);
        appDelegate.onConfigurationChanged(newConfig);
    }

    @Override
    public void attachBaseContext(Context context) {
        Log.i(LOG_TAG, "StageApplication attachBaseContext called");
        if (context == null) {
            Log.e(LOG_TAG, "context is null");
            return;
        }
        super.attachBaseContext(StageApplicationDelegate.attachLanguageContext(context));
    }

    /**
     * Preload the abc file.
     *
     * @param moduleName Preload the name of module.
     * @param abilityName Preload the name of ability.
     */
    public static void preloadEtsModule(String moduleName, String abilityName) {
        if (moduleName == null || moduleName.isEmpty()) {
            Log.e(LOG_TAG, "moduleName is null");
            return;
        }
        if (abilityName == null || abilityName.isEmpty()) {
            Log.e(LOG_TAG, "abilityName is null");
            return;
        }
        StageApplicationDelegate.nativePreloadModule(moduleName, abilityName);
    }
}
