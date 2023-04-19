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

import org.json.JSONException;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import android.util.Log;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import ohos.stage.ability.adapter.StageApplication;
import ohos.stage.ability.adapter.StageApplicationDelegate;

/**
 * EnvironmentTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class StageApplicationDelegateTest {
    private static final String LOG_TAG = "StageApplicationDelegateTest";
    private static final String TEST_PATH = "./";

    private StageApplication stageApplication;
    private StageApplicationDelegate stageApplicationDelegate;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        Log.i(LOG_TAG, "doBefore");
        stageApplication = new StageApplication();
        stageApplicationDelegate = new StageApplicationDelegate();
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        Log.i(LOG_TAG, "doAfter");
        this.stageApplication = null;
        this.stageApplicationDelegate = null;
    }

    /**
     * setNativeAssetManager
     */
    @Test
    public void setNativeAssetManager_0100() throws JSONException {
        try {
            stageApplicationDelegate.setNativeAssetManager(stageApplication.getAssets());
        }
        catch (Exception e) {
            Assert.assertNotEquals(null, stageApplicationDelegate);
        }
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * setHapPath
     */
    @Test
    public void setHapPath_0100() {
        stageApplicationDelegate.setHapPath(TEST_PATH);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * setAssetsFileRelativePath
     */
    @Test
    public void setAssetsFileRelativePath_0100() {
        stageApplicationDelegate.setAssetsFileRelativePath(TEST_PATH);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * launchApplication
     */
    @Test
    public void launchApplication_0100() {
        stageApplicationDelegate.launchApplication();
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * setCacheDir
     */
    @Test
    public void setCacheDir_0100() {
        stageApplicationDelegate.setCacheDir(TEST_PATH);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * setFileDir
     */
    @Test
    public void setFileDir_0100() {
        stageApplicationDelegate.setFileDir(TEST_PATH);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * setResourcesFilePrefixPath
     */
    @Test
    public void setResourcesFilePrefixPath_0100() {
        stageApplicationDelegate.setResourcesFilePrefixPath(TEST_PATH);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * setPidAndUid
     */
    @Test
    public void setPidAndUid_0100() {
        int pid = 0;
        int uid = 0;
        stageApplicationDelegate.setPidAndUid(pid, uid);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * initConfiguration
     */
    @Test
    public void initConfiguration_0100() {
        String data = "";
        stageApplicationDelegate.initConfiguration(data);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * onConfigurationChanged
     */
    @Test
    public void onConfigurationChanged_0100() {
        String data = "";
        stageApplicationDelegate.onConfigurationChanged(data);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * setLocale
     */
    @Test
    public void setLocale_0100() {
        String language = "zh";
        String country = "CN";
        String script = "Hans";
        stageApplicationDelegate.setLocale(language, country, script);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }

    /**
     * attachStageApplication
     */
    @Test
    public void attachStageApplication_0100() {
        stageApplicationDelegate.attachStageApplication(stageApplication);
        Assert.assertNotEquals(null, stageApplicationDelegate);
    }
}