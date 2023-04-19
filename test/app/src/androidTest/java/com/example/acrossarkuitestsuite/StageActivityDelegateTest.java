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

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import android.util.Log;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import ohos.stage.ability.adapter.StageActivityDelegate;

/**
 * EnvironmentTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class StageActivityDelegateTest {
    private static String INSTANCE_NAME = "instanceName";
    private StageActivityDelegate stageActivityDelegate;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        Log.i("StageActivityDelegateTest", "doBefore");
        stageActivityDelegate = new StageActivityDelegate();
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        Log.i("StageActivityDelegateTest", "doAfter");
        this.stageActivityDelegate = null;
    }

    /**
     * attachStageActivity
     */
    @Test
    public void attachStageActivity_0100() {
        stageActivityDelegate.attachStageActivity(INSTANCE_NAME, null);
        Assert.assertNotEquals(null, stageActivityDelegate);
    }

    /**
     * dispatchOnCreate
     */
    @Test
    public void dispatchOnCreate_0100() {
        stageActivityDelegate.dispatchOnCreate(INSTANCE_NAME);
        Assert.assertNotEquals(null, stageActivityDelegate);
    }

    /**
     * dispatchOnDestroy
     */
    @Test
    public void dispatchOnDestroy_0100() {
        stageActivityDelegate.dispatchOnDestroy(INSTANCE_NAME);
        Assert.assertNotEquals(null, stageActivityDelegate);
    }

    /**
     * dispatchOnNewWant
     */
    @Test
    public void dispatchOnNewWant_0100() {
        stageActivityDelegate.dispatchOnNewWant(INSTANCE_NAME);
        Assert.assertNotEquals(null, stageActivityDelegate);
    }

    /**
     * dispatchOnForeground
     */
    @Test
    public void dispatchOnForeground_0100() {
        stageActivityDelegate.dispatchOnForeground(INSTANCE_NAME);
        Assert.assertNotEquals(null, stageActivityDelegate);
    }

    /**
     * dispatchOnBackground
     */
    @Test
    public void dispatchOnBackground_0100() {
        stageActivityDelegate.dispatchOnBackground(INSTANCE_NAME);
        Assert.assertNotEquals(null, stageActivityDelegate);
    }

    /**
     * setWindowView
     */
    @Test
    public void setWindowView_0100() {
        stageActivityDelegate.setWindowView(INSTANCE_NAME, null);
        Assert.assertNotEquals(null, stageActivityDelegate);
    }
}