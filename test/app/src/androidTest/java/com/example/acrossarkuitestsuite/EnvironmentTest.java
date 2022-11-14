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

package com.example.acrossarkuitestsuite;

import android.content.Context;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.capability.environment.EnvironmentAosp;

import static org.junit.Assert.assertEquals;

/**
 * EnvironmentTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class EnvironmentTest {
    private AceEnv aceEnv;
    private Context context;
    private EnvironmentAosp environment;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        this.aceEnv = AceEnv.getInstance();
        environment = new EnvironmentAosp(this.context);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.context = null;
        this.environment = null;
    }

    /**
     * getAccessibilityEnabled
     */
    @Test
    public void getAccessibilityEnabled_0100() {
        String result = environment.getAccessibilityEnabled();
        assertEquals(result, "false");
    }
}