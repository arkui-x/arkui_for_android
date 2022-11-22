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
import ohos.ace.adapter.IAceOnCallResourceMethod;
import ohos.ace.adapter.capability.texture.AceTexture;
import ohos.ace.adapter.capability.texture.IAceTexture;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import java.util.HashMap;
import java.util.Map;

/**
 * TextureTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class TextureTest {
    private static final String LOG_TAG = "TextureTest";
    private AceTexture aceTexture;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        Map<String, String> params = new HashMap<>();
        params.put("initParam", "registerSurface");
        aceTexture = new AceTexture(0, new IAceTexture() {
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
        }, null, params);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        if (aceTexture != null) {
            this.aceTexture.release();
            this.aceTexture = null;
        }
    }

    /**
     * Get the call method map.
     */
    @Test
    public void getCallMethod_0100() {
        if (aceTexture != null) {
            Map<String, IAceOnCallResourceMethod> params = aceTexture.getCallMethod();
            assertNotNull(params);
        } else {
            assertNotNull(aceTexture);
        }
    }

    /**
     * Set the size of texture.
     */
    @Test
    public void setTextureSize_0100() {
        if (aceTexture != null) {
            Map<String, String> params = new HashMap<>();
            params.put("textureWidth", "50");
            params.put("textureHeight", "50");
            params.put("setDefaultSize", "1");
            String result = aceTexture.setTextureSize(params);
            assertEquals(result, "success");
        } else {
            assertNotNull(aceTexture);
        }
    }

    /**
     * Set the size of texture.
     */
    @Test
    public void setTextureSize_0200() {
        if (aceTexture != null) {
            Map<String, String> params = new HashMap<>();
            String result = aceTexture.setTextureSize(params);
            assertEquals(result, "false");
        } else {
            assertNotNull(aceTexture);
        }
    }

    /**
     * Get the surface texture.
     */
    @Test
    public void getSurfaceTexture_0100() {
        if (aceTexture != null) {
            SurfaceTexture surfaceTexture = aceTexture.getSurfaceTexture();
            assertNotNull(surfaceTexture);
        } else {
            assertNotNull(aceTexture);
        }
    }

    /**
     * Get the surface.
     */
    @Test
    public void getSurface_0100() {
        if (aceTexture != null) {
            Surface surface = aceTexture.getSurface();
            assertNotNull(surface);
        } else {
            assertNotNull(aceTexture);
        }
    }

    /**
     * Get the resource id.
     */
    @Test
    public void getId_0100() {
        if (aceTexture != null) {
            long id = aceTexture.getId();
            assertEquals(id, 0);
        } else {
            assertNotNull(aceTexture);
        }
    }
}