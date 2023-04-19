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

import ohos.stage.ability.adapter.InstanceIdGenerator;

/**
 * EnvironmentTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class InstanceIdGeneratorTest {
    private static final String LOG_TAG = "InstanceIdGeneratorTest";

    private InstanceIdGenerator instanceIdGenerator;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        Log.i(LOG_TAG, "doBefore");
        instanceIdGenerator = new InstanceIdGenerator();
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        Log.i(LOG_TAG, "doAfter");
        this.instanceIdGenerator = null;
    }

    /**
     * getAndIncrement
     */
    @Test
    public void InstanceIdGenerator_0100() {
        int ret = instanceIdGenerator.getAndIncrement();
        Assert.assertNotEquals(0, ret);
    }

    /**
     * get
     */
    @Test
    public void InstanceIdGenerator_0200() {
        int ret = instanceIdGenerator.get();
        Assert.assertNotEquals(100, ret);
    }
}