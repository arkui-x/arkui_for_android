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

import java.util.HashMap;
import java.util.Map;

import ohos.ace.adapter.AceResourceRegister;
import ohos.ace.adapter.capability.texture.AceTexturePluginAosp;
import ohos.ace.adapter.capability.video.AceVideoPluginAosp;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;

/**
 * VideoPluginTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class VideoPluginTest {
    private Context context;
    private AceVideoPluginAosp aceVideoPluginAosp;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        aceVideoPluginAosp = AceVideoPluginAosp.createRegister(context, "VideoPluginTest");
        AceResourceRegister aceResourceRegister = new AceResourceRegister();
        AceTexturePluginAosp aceTexturePluginAosp = AceTexturePluginAosp.createRegister(null);
        aceTexturePluginAosp.create(new HashMap<>());
        aceResourceRegister.registerPlugin(aceTexturePluginAosp);
        aceVideoPluginAosp.setEventCallback(aceResourceRegister, null);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        context = null;
        aceVideoPluginAosp.release();
        aceVideoPluginAosp = null;
    }

    /**
     * Create resource by param
     */
    @Test
    public void create_0100() {
        Map<String, String> param = new HashMap<>();
        param.put("texture", "0");
        long result = aceVideoPluginAosp.create(param);
        assertNotEquals(result, -1);
    }

    /**
     * Create resource by param
     */
    @Test
    public void create_0200() {
        Map<String, String> param = new HashMap<>();
        param.put("texture2", "0");
        long result = aceVideoPluginAosp.create(param);
        assertEquals(result, -1);
    }

    /**
     * Create resource by param
     */
    @Test
    public void create_0300() {
        Map<String, String> param = new HashMap<>();
        param.put("texture", "1");
        long result = aceVideoPluginAosp.create(param);
        assertEquals(result, -1);
    }

    /**
     * Create resource by param
     */
    @Test
    public void create_0400() {
        Map<String, String> param = new HashMap<>();
        param.put("texture", "1xx");
        long result = aceVideoPluginAosp.create(param);
        assertEquals(result, -1);
    }
}