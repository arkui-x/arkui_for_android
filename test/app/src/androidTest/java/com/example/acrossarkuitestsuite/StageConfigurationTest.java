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

package com.example.acrossarkuitestsuite;

import static android.content.res.Configuration.UI_MODE_NIGHT_YES;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import android.content.res.Configuration;
import android.util.Log;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import ohos.stage.ability.adapter.StageConfiguration;

/**
 * EnvironmentTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class StageConfigurationTest {
    private static final String LOG_TAG = "StageConfigurationTest";
    private static final String COLOR_MODE_KEY = "ohos.system.colorMode";
    private static final String COLOR_MODE_DARK = "dark";

    private StageConfiguration stageConfiguration;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        Log.i(LOG_TAG, "doBefore");
        stageConfiguration = new StageConfiguration();
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        Log.i(LOG_TAG, "doAfter");
        this.stageConfiguration = null;
    }

    /**
     * convertConfiguration
     */
    @Test
    public void StageConfiguration_0100() throws JSONException {
        Configuration configuration = new Configuration();
        configuration.uiMode = UI_MODE_NIGHT_YES;
        JSONObject json = stageConfiguration.convertConfiguration(configuration);
        Assert.assertEquals(COLOR_MODE_DARK, json.getString(COLOR_MODE_KEY));
    }
}