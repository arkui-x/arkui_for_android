/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

import android.app.NotificationManager;
import android.content.Context;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.media.AudioManager;
import android.Manifest;
import android.content.pm.PackageManager;

import ohos.ace.adapter.ALog;

/**
 * VibratorPluginAosp is an add-on for handling vibrator.
 *
 * @since 1
 */
public class VibratorAosp extends VibratorPluginBase {
    private static final String LOG_TAG = "Ace_Vibrator";
    private static final String VIBRATOR_TYPE_INVALID = "vibrator.type.invalid";
    private static final String VIBRATOR_TYPE_LONG_PRESS_LIGHT = "haptic.long_press.light";
    private static final String VIBRATOR_TYPE_LONG_PRESS_MEDIUM = "haptic.long_press.medium";
    private static final String VIBRATOR_TYPE_SLIDE_LIGHT = "haptic.slide.light";
    private static final String VIBRATOR_TYPE_DRAG = "haptic.drag";
    private static final int HAPTIC_SLIDE_LIGHT_AMPLITUDE = 255;
    private static final int HAPTIC_LONG_PRESS_AMPLITUDE = 255;
    private static final int HAPTIC_LONG_PRESS_MEDIUM_AMPLITUDE = 255;
    private static final int HAPTIC_DRAG_AMPLITUDE = 255;
    private static final int HAPTIC_MAX_AMPLITUDE = 255;
    private static final int HAPTIC_AMPLITUDE_MIN = 0;
    private static final int HAPTIC_OTHER_AMPLITUDE = 0;
    private static final long HAPTIC_SLIDE_LIGHT_DURATION = 10L;
    private static final long HAPTIC_LONG_PRESS_DURATION = 80L;
    private static final long HAPTIC_LONG_PRESS_MEDIUM_DURATION = 80L;
    private static final long HAPTIC_DRAG_DURATION = 3L;
    private static final long HAPTIC_OTHER_DURATION = 0L;
    private static final float MAX_HAPTIC_INTENSITY = 100.0f;
    private static final int MIN_AMPLITUDE = 1;
    private static final int MAX_AMPLITUDE = 255;

    private Vibrator vibrator = null;
    private Context context = null;
    private AudioManager audioManager = null;

    /**
     * constructor of VibratorPlugin in AOSP platform
     *
     * @param context the context of application
     */
    public VibratorAosp(Context context) {
        this.context = context;
        if (context != null) {
            Object temp = context.getSystemService(Context.VIBRATOR_SERVICE);
            if (temp instanceof Vibrator) {
                this.vibrator = (Vibrator) temp;
            }

            Object audioService = context.getSystemService(Context.AUDIO_SERVICE);
            if (audioService instanceof AudioManager) {
                audioManager = (AudioManager) audioService;
            }
        }

        nativeInit();
    }

    @Override
    public void vibrate(int duration) {
        if (vibrator == null) {
            ALog.e(LOG_TAG, "Vibrator service not available");
            return;
        }

        try {
            if (!hasVibratePermission()) {
                return;
            }
            if (duration < 0) {
                return;
            }
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                vibrator.vibrate(VibrationEffect.createOneShot(duration, VibrationEffect.DEFAULT_AMPLITUDE));
            } else {
                vibrator.vibrate(duration);
            }
        } catch (SecurityException | IllegalArgumentException | UnsupportedOperationException e) {
            ALog.e(LOG_TAG, "Vibration failed");
        }
    }

    @Override
    public void vibrate(String effectId) {
        if (vibrator == null) {
            ALog.e(LOG_TAG, "Vibrator service not available");
            return;
        }

        try {
            if (!hasVibratePermission()) {
                return;
            }
            String mappedEffectId = mapEffectId(effectId);
            if (!isSupportedEffect(mappedEffectId)) {
                return;
            }
            long duration = getDuration(mappedEffectId);
            int amplitude = getAmplitude(mappedEffectId);
            if (duration < 0) {
                return;
            }
            if (amplitude != VibrationEffect.DEFAULT_AMPLITUDE &&
                    (amplitude < HAPTIC_AMPLITUDE_MIN || amplitude > HAPTIC_MAX_AMPLITUDE)) {
                return;
            }
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                vibrator.vibrate(VibrationEffect.createOneShot(duration, amplitude));
            } else {
                vibrator.vibrate(duration);
            }
        } catch (SecurityException | IllegalArgumentException e) {
            ALog.e(LOG_TAG, "Vibrate effect failed");
        }
    }

    @Override
    public void vibrate(String effectId, float intensity) {
        if (vibrator == null) {
            ALog.e(LOG_TAG, "Vibrator service not available");
            return;
        }

        if (!hasVibratePermission()) {
            return;
        }

        if (!canVibrate() || isSilentMode() || isDoNotDisturbMode()) {
            return;
        }

        try {
            float safeIntensity = Math.max(0.0f, Math.min(MAX_HAPTIC_INTENSITY, intensity));
            int amplitude = Math.max(MIN_AMPLITUDE,
                    Math.round((safeIntensity / MAX_HAPTIC_INTENSITY) * MAX_AMPLITUDE));
            String mappedEffectId = mapEffectId(effectId);
            if (!isSupportedEffect(mappedEffectId)) {
                return;
            }
            long duration = getDuration(mappedEffectId);
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                vibrator.vibrate(VibrationEffect.createOneShot(duration, amplitude));
            } else {
                vibrator.vibrate(duration);
            }
        } catch (SecurityException e) {
            ALog.e(LOG_TAG, "Has no vibrate permission");
        }
    }

    private boolean canVibrate() {
        return (vibrator != null && vibrator.hasVibrator());
    }

    private String mapEffectId(String effectId) {
        switch (effectId) {
            case "slide":
            case "slide.light":
            case "haptic.slide":
                return VIBRATOR_TYPE_SLIDE_LIGHT;
            case "longPress.light":
                return VIBRATOR_TYPE_LONG_PRESS_LIGHT;
            case "haptic.long_press_medium":
                return VIBRATOR_TYPE_LONG_PRESS_MEDIUM;
            case "haptic.drag":
                return VIBRATOR_TYPE_DRAG;
            default:
                return VIBRATOR_TYPE_INVALID;
        }
    }

    /**
     * Get vibration duration based on effectId
     *
     * @param effectId The effect ID
     * @return Vibration duration in milliseconds, returns 0 by default
     */
    private long getDuration(String effectId) {
        switch (effectId) {
            case VIBRATOR_TYPE_SLIDE_LIGHT:
                return HAPTIC_SLIDE_LIGHT_DURATION;
            case VIBRATOR_TYPE_LONG_PRESS_LIGHT:
                return HAPTIC_LONG_PRESS_DURATION;
            case VIBRATOR_TYPE_LONG_PRESS_MEDIUM:
                return HAPTIC_LONG_PRESS_MEDIUM_DURATION;
            case VIBRATOR_TYPE_DRAG:
                return HAPTIC_DRAG_DURATION;
            default:
                return HAPTIC_OTHER_DURATION;
        }
    }

    /**
     * Get enable status based on effectId
     *
     * @param effectId The effect ID
     * @return Whether it is enabled, returns false by default
     */
    private boolean isSupportedEffect(String effectId) {
        switch (effectId) {
            case VIBRATOR_TYPE_SLIDE_LIGHT:
            case VIBRATOR_TYPE_LONG_PRESS_LIGHT:
            case VIBRATOR_TYPE_LONG_PRESS_MEDIUM:
            case VIBRATOR_TYPE_DRAG:
                return true;
            default:
                return false;
        }
    }

    /**
     * Get intensity value based on effectId
     *
     * @param effectId The effect ID
     * @return Intensity value, returns 0 by default
     */
    private int getAmplitude(String effectId) {
        switch (effectId) {
            case VIBRATOR_TYPE_SLIDE_LIGHT:
                return HAPTIC_SLIDE_LIGHT_AMPLITUDE;
            case VIBRATOR_TYPE_LONG_PRESS_LIGHT:
                return HAPTIC_LONG_PRESS_AMPLITUDE;
            case VIBRATOR_TYPE_LONG_PRESS_MEDIUM:
                return HAPTIC_LONG_PRESS_MEDIUM_AMPLITUDE;
            case VIBRATOR_TYPE_DRAG:
                return HAPTIC_DRAG_AMPLITUDE;
            default:
                return HAPTIC_OTHER_AMPLITUDE;
        }
    }

    /**
     * Checks if VIBRATE permission is granted.
     *
     * @return true if permission granted, false otherwise
     */
    private boolean hasVibratePermission() {
        try {
            if (context == null) {
                return false;
            }
            int result = context.checkCallingOrSelfPermission(android.Manifest.permission.VIBRATE);
            return (result == PackageManager.PERMISSION_GRANTED);
        } catch (NullPointerException e) {
            ALog.e(LOG_TAG, "hasVibratePermission: NullPointerException: NullPointerException");
            return false;
        } catch (SecurityException e) {
            ALog.e(LOG_TAG, "hasVibratePermission: SecurityException: SecurityException");
            return false;
        } catch (IllegalArgumentException e) {
            ALog.e(LOG_TAG, "hasVibratePermission: IllegalArgumentException: IllegalArgumentException");
            return false;
        }
    }

    private boolean isSilentMode() {
        if (audioManager == null) {
            return false;
        }
        return audioManager.getRingerMode() == AudioManager.RINGER_MODE_SILENT;
    }

    private boolean isDoNotDisturbMode() {
        if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.M) {
            return false;
        }
        if (context == null) {
            return false;
        }
        Object service = context.getSystemService(Context.NOTIFICATION_SERVICE);
        if (!(service instanceof NotificationManager)) {
            return false;
        }
        NotificationManager notificationManager = (NotificationManager) service;
        int interruptionFilter = notificationManager.getCurrentInterruptionFilter();
        return interruptionFilter == NotificationManager.INTERRUPTION_FILTER_NONE
                || interruptionFilter == NotificationManager.INTERRUPTION_FILTER_PRIORITY
                || interruptionFilter == NotificationManager.INTERRUPTION_FILTER_UNKNOWN;
    }
}
