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
 */
public class StageConfiguration {
    private static final String LOG_TAG = "StageConfiguration";

    private static final String COLOR_MODE_KEY = "ohos.system.colorMode";

    private static final String COLOR_MODE_LIGHT = "light";

    private static final String COLOR_MODE_DARK = "dark";

    private static final String ORI_MODE_KEY = "ohos.application.direction";

    private static final String ORI_MODE_LANDSCAPE = "vertical";

    private static final String ORI_MODE_PORTRAIT = "horizontal";

    private static final String EMPTY_VALUE = "";

    /**
     * Convert configuration.
     *
     * @param config the configuration.
     * @return the json object.
     */
    public static JSONObject convertConfiguration(Configuration config) {
        JSONObject json = new JSONObject();
        try {
            int currentNightMode = config.uiMode & Configuration.UI_MODE_NIGHT_MASK;
            switch (currentNightMode) {
                case Configuration.UI_MODE_NIGHT_NO:
                    json.put(COLOR_MODE_KEY, COLOR_MODE_LIGHT);
                    break;
                case Configuration.UI_MODE_NIGHT_YES:
                    json.put(COLOR_MODE_KEY, COLOR_MODE_DARK);
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
        } catch (JSONException ignored) {
            Log.e(LOG_TAG, "convertConfiguration parse Configuration failed");
            return json;
        }
        Log.i(LOG_TAG, "convertConfiguration json:" + json.toString());
        return json;
    }
}
