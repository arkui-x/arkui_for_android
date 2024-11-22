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

import android.content.res.Configuration;
import android.util.Log;
import org.json.JSONObject;
import org.json.JSONException;

/**
 * This class is responsible for the stage model configuration transformation.
 *
 * @since 1
 */
public class StageConfiguration {
    private static final String LOG_TAG = "StageConfiguration";

    private static final String COLOR_MODE_KEY = "ohos.system.colorMode";

    private static final String LANGUAGE_MODE_KEY = "ohos.system.language";

    private static final String COLOR_MODE_LIGHT = "light";

    private static final String COLOR_MODE_DARK = "dark";

    private static final String ORI_MODE_KEY = "ohos.application.direction";

    private static final String DEVICE_TYPE = "const.build.characteristics";

    private static final String ORI_MODE_LANDSCAPE = "horizontal";

    private static final String ORI_MODE_PORTRAIT = "vertical";

    private static final String EMPTY_VALUE = "";

    private static final String DENSITY_KEY = "ohos.application.densitydpi";

    private static final String DEVICE_TYPE_PHONE = "Phone";

    private static final String DEVICE_TYPE_TABLET = "Tablet";

    /**
     * Convert configuration.
     *
     * @param config the configuration.
     * @return the json object.
     */
    public static JSONObject convertConfiguration(Configuration config, double diagonalSize) {
        JSONObject json = new JSONObject();
        try {
            int currentNightMode = config.uiMode & Configuration.UI_MODE_NIGHT_MASK;
            switch (currentNightMode) {
                case Configuration.UI_MODE_NIGHT_YES:
                    json.put(COLOR_MODE_KEY, COLOR_MODE_DARK);
                    break;
                case Configuration.UI_MODE_NIGHT_NO:
                    json.put(COLOR_MODE_KEY, COLOR_MODE_LIGHT);
                    break;
                default:
                    json.put(COLOR_MODE_KEY, EMPTY_VALUE);
                    break;
            }
            int orientationMode = config.orientation;
            switch (orientationMode) {
                case Configuration.ORIENTATION_LANDSCAPE:
                    json.put(ORI_MODE_KEY, ORI_MODE_LANDSCAPE);
                    break;
                case Configuration.ORIENTATION_PORTRAIT:
                    json.put(ORI_MODE_KEY, ORI_MODE_PORTRAIT);
                    break;
                default:
                    json.put(ORI_MODE_KEY, EMPTY_VALUE);
                    break;
            }
            int den = config.densityDpi;
            json.put(DENSITY_KEY, String.valueOf(den));
            json.put(LANGUAGE_MODE_KEY, config.locale);
            int minScreenWidth = config.smallestScreenWidthDp;
            if (minScreenWidth == Configuration.SMALLEST_SCREEN_WIDTH_DP_UNDEFINED) {
                if (diagonalSize <= 6.9 && diagonalSize >= 0) {
                    json.put(DEVICE_TYPE, DEVICE_TYPE_PHONE);
                } else if (diagonalSize > 6.9) {
                    json.put(DEVICE_TYPE, DEVICE_TYPE_TABLET);
                }
            } else if (minScreenWidth >= 600) {
                json.put(DEVICE_TYPE, DEVICE_TYPE_TABLET);
            } else {
                json.put(DEVICE_TYPE, DEVICE_TYPE_PHONE);
            }
        } catch (JSONException ignored) {
            Log.e(LOG_TAG, "convertConfiguration parse Configuration failed");
            return json;
        }
        Log.i(LOG_TAG, "convertConfiguration json:" + json.toString());
        return json;
    }
}
