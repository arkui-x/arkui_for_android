/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
package ohos.ace.adapter;

import android.content.Context;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.WindowManager;
import android.util.Log;

/**
 * The type Display info.
 *
 * @since 2023-08-06
 */
public class DisplayInfo {
    private static final String TAG = "DisplayInfo";
    private static volatile DisplayInfo sInstance;

    private WindowManager mWindowManager;

    private DisplayInfo() {
        Log.d(TAG, "DisplayInfo created.");
    }

    /**
     * Gets instance.
     *
     * @return the instance
     */
    public static final DisplayInfo getInstance() {
        if (sInstance == null) {
            synchronized (DisplayInfo.class) {
                if (sInstance == null) {
                    sInstance = new DisplayInfo();
                }
            }
        }

        return sInstance;
    }

    /**
     * Sets context.
     *
     * @param context the context
     */
    public void setContext(Context context) {
        mWindowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        nativeSetupDisplayInfo();
    }

    /**
     * Gets display id.
     *
     * @return the display id
     */
    public int getDisplayId() {
        Log.d(TAG, "getDisplayId called.");
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        return defaultDisplay.getDisplayId();
    }

    /**
     * Gets orentation.
     *
     * @return the orentation
     */
    public int getOrentation() {
        Log.d(TAG, "getOrentation called.");
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        return defaultDisplay.getRotation();
    }

    /**
     * Gets width.
     *
     * @return the width
     */
    public int getWidth() {
        Log.d(TAG, "getWidth called.");
        DisplayMetrics metrics = new DisplayMetrics();
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        defaultDisplay.getRealMetrics(metrics);
        return metrics.widthPixels;
    }

    /**
     * Gets height.
     *
     * @return the height
     */
    public int getHeight() {
        Log.d(TAG, "getHeight called.");
        DisplayMetrics metrics = new DisplayMetrics();
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        defaultDisplay.getRealMetrics(metrics);
        return metrics.heightPixels;
    }

    /**
     * Gets refresh rate.
     *
     * @return the refresh rate
     */
    public float getRefreshRate() {
        Log.d(TAG, "getRefreshRate called.");
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        return defaultDisplay.getRefreshRate();
    }

    /**
     * Gets density pixels.
     *
     * @return the density pixels.
     */
    public float getDensityPixels() {
        Log.d(TAG, "getDensityPixels called.");
        DisplayMetrics metrics = new DisplayMetrics();
        mWindowManager.getDefaultDisplay().getMetrics(metrics);
        return metrics.density;
    }

    /**
     * Gets density dpi.
     *
     * @return the density dpi.
     */
    public int getDensityDpi() {
        Log.d(TAG, "getDensityDpi called.");
        DisplayMetrics metrics = new DisplayMetrics();
        mWindowManager.getDefaultDisplay().getMetrics(metrics);
        return metrics.densityDpi;
    }

    /**
     * Gets scaled density.
     *
     * @return the scaled density.
     */
    public float getScaledDensity() {
        Log.d(TAG, "getScaledDensity called.");
        DisplayMetrics metrics = new DisplayMetrics();
        mWindowManager.getDefaultDisplay().getMetrics(metrics);
        return metrics.scaledDensity;
    }

    /**
     * Gets dpi in horizontal direction.
     *
     * @return the dpi in horizontal direction.
     */
    public float getXDpi() {
        Log.d(TAG, "getXDpi called.");
        if (mWindowManager == null) {
            return 0.0f;
        }
        DisplayMetrics metrics = new DisplayMetrics();
        mWindowManager.getDefaultDisplay().getMetrics(metrics);
        return metrics.xdpi;
    }

    /**
     * Gets dpi in vertical direction.
     *
     * @return the dpi in vertical direction.
     */
    public float getYDpi() {
        Log.d(TAG, "getYDpi called.");
        if (mWindowManager == null) {
            return 0.0f;
        }
        DisplayMetrics metrics = new DisplayMetrics();
        mWindowManager.getDefaultDisplay().getMetrics(metrics);
        return metrics.ydpi;
    }

    private native void nativeSetupDisplayInfo();
}
