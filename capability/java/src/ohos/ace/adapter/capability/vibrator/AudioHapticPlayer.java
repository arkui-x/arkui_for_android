/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

package ohos.ace.adapter.capability.vibrator;

import android.content.Context;
import android.media.AudioDeviceInfo;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.SoundPool;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import ohos.ace.adapter.ALog;

/**
 * Audio haptic player for loading and playing sonification audio resources.
 *
 * @since 26
 */
public final class AudioHapticPlayer {
    private static final String LOG_TAG = "AudioHapticPlayer";
    private static final int API_28 = 28;
    private static final float DEFAULT_STREAM_VOLUME_DB = 0.0f;

    private int soundId = 0;
    private AudioManager audioManager = null;
    private final Context appContext;
    private final SoundPool soundPool;

    /**
     * this constructor of AudioHapticPlayer
     *
     * @param context the context of application
     */
    public AudioHapticPlayer(Context context) {
        nativeInit();
        this.appContext = context.getApplicationContext();
        Object audioService = context.getSystemService(Context.AUDIO_SERVICE);
        if (audioService instanceof AudioManager) {
            audioManager = (AudioManager) audioService;
        } else {
            ALog.w(LOG_TAG, "AudioManager service not available");
        }

        AudioAttributes attributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_ASSISTANCE_SONIFICATION)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build();
        soundPool = new SoundPool.Builder()
                .setAudioAttributes(attributes)
                .setMaxStreams(1)
                .build();
    }

    /**
     * Preload the audio resource from the specified relative uri.
     *
     * @param uri relative path of the audio file in the application files directory
     */
    public void preload(String uri) {
        if (uri == null || uri.isEmpty()) {
            return;
        }

        try {
            File soundFile = new File(appContext.getFilesDir(), uri);
            String filePath = soundFile.getCanonicalPath();
            soundId = soundPool.load(filePath, 1);
        } catch (IOException e) {
            ALog.e(LOG_TAG, "Failed to preload audio resource");
        }
    }

    /**
     * Play the preloaded audio resource with the specified volume.
     *
     * @param volume playback volume in the range from 0.0 to 1.0
     */
    public void play(float volume) {
        if (soundId == 0) {
            ALog.e(LOG_TAG, "Sound not loaded");
            return;
        }

        float safeVolume = Math.max(0.0f, Math.min(1.0f, volume));
        soundPool.play(soundId, safeVolume, safeVolume, 1, 0, 1.0f);
    }

    /**
     * Release the loaded audio resource and sound pool instance.
     */
    public void release() {
        if (soundId != 0) {
            soundPool.unload(soundId);
            soundId = 0;
        }
        soundPool.release();
    }

    /**
     * Get the current ring stream volume in decibels.
     *
     * @return current ring stream volume in decibels
     */
    public float getStreamVolumeDb() {
        if (audioManager == null) {
            ALog.e(LOG_TAG, "AudioManager not available");
            return DEFAULT_STREAM_VOLUME_DB;
        }

        int currentVolume = audioManager.getStreamVolume(AudioManager.STREAM_RING);
        if (currentVolume <= 0) {
            ALog.e(LOG_TAG, "Current volume is zero or negative: " + currentVolume);
            return DEFAULT_STREAM_VOLUME_DB;
        }

        return queryStreamVolumeDb(currentVolume);
    }

    private Float queryStreamVolumeDb(int volumeIndex) {
        try {
            Method method = AudioManager.class.getMethod(
                    "getStreamVolumeDb", int.class, int.class, int.class);
            Object value = method.invoke(
                    audioManager, AudioManager.STREAM_RING, volumeIndex, AudioDeviceInfo.TYPE_BUILTIN_SPEAKER);
            if (value instanceof Number) {
                float currentDb = ((Number) value).floatValue();
                return Float.isFinite(currentDb) ? currentDb : DEFAULT_STREAM_VOLUME_DB;
            }
        } catch (NoSuchMethodException e) {
            ALog.e(LOG_TAG, "getStreamVolumeDb method not found, the method requires API level 28 or higher.");
        } catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException | SecurityException e) {
            ALog.e(LOG_TAG, "Exception invoking getStreamVolumeDb");
        }
        return DEFAULT_STREAM_VOLUME_DB;
    }

    private native void nativeInit();
}
