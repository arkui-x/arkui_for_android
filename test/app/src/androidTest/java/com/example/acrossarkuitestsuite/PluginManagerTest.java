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
import ohos.ace.adapter.capability.plugin.PluginManager;

import static org.junit.Assert.assertNotNull;

/**
 * PluginManagerTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class PluginManagerTest {
    private AceEnv aceEnv;
    private Context context;
    private PluginManager pluginManager;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        this.aceEnv = AceEnv.getInstance();
        pluginManager = new PluginManager(this.context);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.context = null;
        this.pluginManager = null;
    }

    /**
     * initPlugin
     */
    @Test
    public void initPlugin_0100() {
        if (pluginManager != null) {
            pluginManager.initPlugin("com.example.acrossarkuitestsuite");
        } else {
            assertNotNull(pluginManager);
        }
    }
}