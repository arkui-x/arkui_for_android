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
import android.graphics.SurfaceTexture;
import android.view.Surface;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.HashMap;
import java.util.Map;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.capability.video.AceVideoAosp;

import static org.junit.Assert.assertEquals;

/**
 * VideoTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class VideoTest {
    private Context context;
    private AceVideoAosp aceVideoAosp;
    private Map<String, String> param;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        aceVideoAosp = new AceVideoAosp(1, "MainAbility",
            new Surface(new SurfaceTexture(0)), context, (s, s1) -> {
                ALog.i("video", "video test callback");
            });
        param = new HashMap<>();
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        aceVideoAosp.release();
    }

    private void initMediaPlayer() {
        param.put("src", "/media/show.mp4");
        aceVideoAosp.initMediaPlayer(param);
    }

    /**
     * init media player
     */
    @Test
    public void initMediaPlayer_0100() {
        String result = aceVideoAosp.initMediaPlayer(null);
        assertEquals("fail", result);
    }

    /**
     * init media player
     */
    @Test
    public void initMediaPlayer_0200() {
        String result = aceVideoAosp.initMediaPlayer(param);
        assertEquals("fail", result);
    }

    /**
     * init media player
     */
    @Test
    public void initMediaPlayer_0300() {
        param.put("src", "/media/show_none.mp4");
        String result = aceVideoAosp.initMediaPlayer(param);
        assertEquals("fail", result);
    }

    /**
     * init media player
     */
    @Test
    public void initMediaPlayer_0400() {
        param.put("src", "/media/show.mp4");
        String result = aceVideoAosp.initMediaPlayer(param);
        assertEquals("success", result);
    }

    /**
     * start player
     */
    @Test
    public void start_0100() {
        initMediaPlayer();
        String result = aceVideoAosp.start(param);
        assertEquals("success", result);
    }

    /**
     * pause player
     */
    @Test
    public void pause_0100() {
        initMediaPlayer();
        String result = aceVideoAosp.pause(param);
        assertEquals("success", result);
    }

    /**
     * seek video
     */
    @Test
    public void seekTo_0100() {
        initMediaPlayer();
        String result = aceVideoAosp.seekTo(null);
        assertEquals("fail", result);
    }

    /**
     * seek video
     */
    @Test
    public void seekTo_0200() {
        initMediaPlayer();
        String result = aceVideoAosp.seekTo(param);
        assertEquals("fail", result);
    }

    /**
     * seek video
     */
    @Test
    public void seekTo_0300() {
        initMediaPlayer();
        param.put("value", "5");
        String result = aceVideoAosp.seekTo(param);
        assertEquals("success", result);
    }

    /**
     * seek video
     */
    @Test
    public void seekTo_0400() {
        initMediaPlayer();
        param.put("value", "5xx");
        String result = aceVideoAosp.seekTo(param);
        assertEquals("fail", result);
    }

    /**
     * set volume
     */
    @Test
    public void setVolume_0100() {
        initMediaPlayer();
        String result = aceVideoAosp.setVolume(null);
        assertEquals("fail", result);
    }

    /**
     * set volume
     */
    @Test
    public void setVolume_0200() {
        initMediaPlayer();
        String result = aceVideoAosp.setVolume(param);
        assertEquals("fail", result);
    }

    /**
     * set volume
     */
    @Test
    public void setVolume_0300() {
        initMediaPlayer();
        param.put("value", "5.55");
        String result = aceVideoAosp.setVolume(param);
        assertEquals("success", result);
    }

    /**
     * set volume
     */
    @Test
    public void setVolume_0400() {
        initMediaPlayer();
        param.put("value", "5xx");
        String result = aceVideoAosp.setVolume(param);
        assertEquals("fail", result);
    }

    /**
     * get video position
     */
    @Test
    public void getPosition_0100() {
        initMediaPlayer();
        param.put("value", "5");
        aceVideoAosp.seekTo(param);
        String result = aceVideoAosp.getPosition(param);
        assertEquals("currentpos=5", result);
    }

    /**
     * enable single looping
     */
    @Test
    public void enableLooping_0100() {
        initMediaPlayer();
        String result = aceVideoAosp.enableLooping(null);
        assertEquals("fail", result);
    }

    /**
     * enable single looping
     */
    @Test
    public void enableLooping_0200() {
        initMediaPlayer();
        param.put("loop", "0");
        String result = aceVideoAosp.enableLooping(param);
        assertEquals("success", result);
    }

    /**
     * enable single looping
     */
    @Test
    public void enableLooping_0300() {
        initMediaPlayer();
        param.put("loop", "1");
        String result = aceVideoAosp.enableLooping(param);
        assertEquals("success", result);
    }

    /**
     * enable single looping
     */
    @Test
    public void enableLooping_0400() {
        initMediaPlayer();
        param.put("loop", "5xx");
        String result = aceVideoAosp.enableLooping(param);
        assertEquals("fail", result);
    }

    /**
     * set video speed
     */
    @Test
    public void setSpeed_0100() {
        initMediaPlayer();
        String result = aceVideoAosp.setSpeed(null);
        assertEquals("fail", result);
    }

    /**
     * set video speed
     */
    @Test
    public void setSpeed_0200() {
        initMediaPlayer();
        String result = aceVideoAosp.setSpeed(param);
        assertEquals("fail", result);
    }

    /**
     * set video speed
     */
    @Test
    public void setSpeed_0300() {
        initMediaPlayer();
        param.put("value", "5.55");
        String result = aceVideoAosp.setSpeed(param);
        assertEquals("success", result);
    }

    /**
     * set video speed
     */
    @Test
    public void setSpeed_0400() {
        initMediaPlayer();
        param.put("value", "5xx");
        String result = aceVideoAosp.setSpeed(param);
        assertEquals("fail", result);
    }
}