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

package ohos.ace.adapter.capability.vibrator;

import android.content.Context;
import android.os.VibrationEffect;
import android.os.Vibrator;

import ohos.ace.adapter.ALog;

/**
 * VibratorPluginAosp is an add-on for handling vibrator.
 *
 * @since 1
 */
public class VibratorPluginAosp extends VibratorPluginBase {
    private static final String LOG_TAG = "Ace_Vibrator";

    private final Vibrator vibrator;

    /**
     * constructor of VibratorPlugin in AOSP platform
     *
     * @param context the context of application
     */
    public VibratorPluginAosp(Context context) {
        if (context != null) {
            Object temp = context.getSystemService(Context.VIBRATOR_SERVICE);
            if (temp instanceof Vibrator) {
                this.vibrator = (Vibrator) temp;
            } else {
                ALog.e(LOG_TAG, "Unable to get VIBRATOR_SERVICE");
                this.vibrator = null;
            }
        } else {
            ALog.e(LOG_TAG, "VibratorPluginAosp context is null");
            this.vibrator = null;
        }

        nativeInit();
    }

    @Override
    public void vibrate(int duration) {
        try {
            if (vibrator != null) {
                vibrator.vibrate(VibrationEffect.createOneShot(duration, VibrationEffect.DEFAULT_AMPLITUDE));
            }
        } catch (SecurityException e) {
            ALog.e(LOG_TAG, "Has no vibrate permission");
        }
    }

    @Override
    public void vibrate(String effectId) {}
}
