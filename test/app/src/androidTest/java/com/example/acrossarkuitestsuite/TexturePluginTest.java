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

import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.capability.texture.AceTexturePluginAosp;
import ohos.ace.adapter.capability.texture.IAceTexture;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import java.util.HashMap;
import java.util.Map;

/**
 * TexturePluginTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class TexturePluginTest {
    private static final String LOG_TAG = "TexturePluginTest";
    private AceTexturePluginAosp aceTexturePluginAosp;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        aceTexturePluginAosp = AceTexturePluginAosp.createRegister(new IAceTexture() {
            @Override
            public void registerTexture(long param, Object object) {
                ALog.i(LOG_TAG, "registerTexture");
            }

            @Override
            public void registerSurface(long param, Object object) {
                ALog.i(LOG_TAG, "registerSurface");
            }

            @Override
            public void markTextureFrameAvailable(long param) {
                ALog.i(LOG_TAG, "markTextureFrameAvailable");
            }

            @Override
            public void unregisterTexture(long param) {
                ALog.i(LOG_TAG, "unregisterTexture");
            }

            @Override
            public void unregisterSurface(long param) {
                ALog.i(LOG_TAG, "unregisterSurface");
            }
        });
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.aceTexturePluginAosp = null;
    }

    /**
     * invoke method of IAceTexture
     */
    @Test
    public void create_0100() {
        Map<String, String> param = new HashMap<>();
        param.put("markTextureFrameAvailable", "0");
        long result = this.aceTexturePluginAosp.create(param);
        assertEquals(result, 0);
    }

    /**
     * invoke method of IAceTexture
     */
    @Test
    public void create_0200() {
        Map<String, String> param = new HashMap<>();
        param.put("markTextureFrameAvailable", "0");
        long result = this.aceTexturePluginAosp.create(param);
        assertNotEquals(result, -1);
    }

    /**
     * get object in AceTexture
     */
    @Test
    public void getObject_0100() {
        Map<String, String> param = new HashMap<>();
        param.put("markTextureFrameAvailable", "0");
        long id = this.aceTexturePluginAosp.create(param);
        Object object = this.aceTexturePluginAosp.getObject(id);
        assertNotNull(object);
    }

    /**
     * get object null in AceTexture
     */
    @Test
    public void getObject_0200() {
        Object object = this.aceTexturePluginAosp.getObject(-1);
        assertNull(object);
    }

    /**
     * release object
     */
    @Test
    public void release_0100() {
        Map<String, String> param = new HashMap<>();
        param.put("markTextureFrameAvailable", "0");
        long id = this.aceTexturePluginAosp.create(param);
        boolean result = this.aceTexturePluginAosp.release(id);
        assertEquals(result, true);
    }

    /**
     * release object
     */
    @Test
    public void release_0200() {
        Map<String, String> param = new HashMap<>();
        param.put("markTextureFrameAvailable", "0");
        long id = this.aceTexturePluginAosp.create(param);
        boolean result = this.aceTexturePluginAosp.release(-1);
        assertEquals(result, false);
    }
}